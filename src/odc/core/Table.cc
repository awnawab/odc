/*
 * (C) Copyright 1996-2018 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "odc/core/Table.h"

#include <functional>
#include <bitset>

#include "eckit/io/AutoCloser.h"
#include "eckit/io/Buffer.h"
#include "eckit/io/MemoryHandle.h"
#include "eckit/types/FixedString.h"

#include "odc/core/DecodeTarget.h"
#include "odc/core/Header.h"
#include "odc/core/MetaData.h"
#include "odc/core/Codec.h"

using namespace eckit;


namespace odc {
namespace core {

//----------------------------------------------------------------------------------------------------------------------

Table::Table(const ThreadSharedDataHandle& dh) :
    dh_(dh) {}

Offset Table::startPosition() const {
    return startPosition_;
}


Offset Table::nextPosition() const {
    return nextPosition_;
}

Length Table::encodedDataSize() const {
    return dataSize_;
}

size_t Table::numRows() const {
    return metadata_.rowsNumber();
}

size_t Table::numColumns() const {
    return metadata_.size();
}

int32_t Table::byteOrder() const {
    return byteOrder_;
}

bool Table::otherByteOrder() const {
    return byteOrder_ != BYTE_ORDER_INDICATOR;
}

const MetaData& Table::columns() const {
    return metadata_;
}

const Properties& Table::properties() const {
    return properties_;
}

Buffer Table::readEncodedData() {

    Buffer data(dataSize_);

    dh_.seek(dataPosition_);
    dh_.read(data, dataSize_);
    return data;
}

void Table::decode(DecodeTarget& target) {

    const MetaData& metadata(columns());
    size_t nrows = metadata.rowsNumber();
    size_t ncols = metadata.size();

    // Create a lookup for the columns by name

    std::map<std::string, size_t> columnLookup;
    std::map<std::string, size_t> lookupSimple;

    for (size_t i = 0; i < ncols; i++) {
        const auto& nm(metadata[i]->name());
        if (!columnLookup.emplace(nm, i).second) {
            std::stringstream ss;
            ss << "Duplicate column '" << nm << "' " << " found in table";
            throw ODBDecodeError(ss.str(), Here());
        }
        lookupSimple.emplace(nm.substr(0, nm.find('@')), i);
    }

    // Loop over the specified output columns, and select the correct ones for decoding.

    std::vector<char> visitColumn(ncols, false);
    std::vector<api::StridedData*> facades(ncols, 0); // TODO: Do we want to do a copy, rather than point to StridedData*?

    ASSERT(target.columns().size() == target.dataFacades().size());
    ASSERT(target.columns().size() <= ncols);

    for (size_t i = 0; i < target.columns().size(); i++) {

        const auto& nm(target.columns()[i]);
        auto it = columnLookup.find(nm);
        if (it == columnLookup.end()) it = lookupSimple.find(nm);
        if (it == lookupSimple.end()) {
            std::stringstream ss;
            ss << "Column '" << nm << "' not found in ODB";
            throw ODBDecodeError(ss.str(), Here());
        }

        size_t pos = it->second;
        if (visitColumn[pos]) {
            std::stringstream ss;
            ss << "Duplicated column '" << nm << "' in decode specification";
            throw ODBDecodeError(ss.str(), Here());
        }

        visitColumn[pos] = true;
        facades[pos] = &target.dataFacades()[i];
        ASSERT(target.dataFacades()[i].nelem() >= nrows);
    }

    // Read the data in in bulk for this table

    const Buffer readBuffer(readEncodedData());
    GeneralDataStream ds(otherByteOrder(), readBuffer);

    std::vector<std::reference_wrapper<Codec>> decoders;
    decoders.reserve(ncols);
    for (auto& col : metadata) {
        decoders.push_back(col->coder());
        decoders.back().get().setDataStream(ds);
    }

    // Do the decoding

    int lastStartCol = 0;
    std::vector<size_t> lastDecoded(ncols, 0);

    for (size_t rowCount = 0; rowCount < nrows; ++rowCount) {

        unsigned char marker[2];
        ds.readBytes(&marker, sizeof(marker));
        int startCol = (marker[0] * 256) + marker[1]; // Endian independant

        if (lastStartCol > startCol) {
            for (int col = startCol; col < lastStartCol; col++) {
                if (visitColumn[col]) {
                    facades[col]->fill(lastDecoded[col], rowCount-1);
                }
            }
        }

        for (int col = startCol; col < long(ncols); col++) {
            if (visitColumn[col]) {
                decoders[col].get().decode(reinterpret_cast<double*>((*facades[col])[rowCount]));
                lastDecoded[col] = rowCount;
            } else {
                decoders[col].get().skip();
            }
        }

        lastStartCol = startCol;
    }

    // And fill in any columns that are incomplete

    for (size_t col = 0; col < ncols; col++) {
        if (lastDecoded[col] < nrows-1) {
            if (visitColumn[col]) {
                facades[col]->fill(lastDecoded[col], nrows-1);
            }
        } else {
            break;
        }
    }
}


std::unique_ptr<Table> Table::readTable(odc::core::ThreadSharedDataHandle& dh) {

    Offset startPosition = dh.position();

    // Check the magic number. If no more data, we are done

    if (!Header::readMagic(dh)) return 0;

    // Load the header

    std::unique_ptr<Table> newTable(new Table(dh));
    Header hdr(newTable->metadata_, newTable->properties_);
    hdr.loadAfterMagic(dh);

    newTable->startPosition_ = startPosition;
    newTable->dataPosition_ = dh.position();
    newTable->dataSize_ = hdr.dataSize();
    newTable->nextPosition_ = dh.position() + newTable->dataSize_;
    newTable->byteOrder_ = hdr.byteOrder();

    // Check that the ODB hasn't been truncated.
    // n.b. Some DataHandles always return 0 (e.g. on a stream), so leth that pass.
    if (newTable->nextPosition_ > dh.estimate() && dh.estimate() != 0) {
        throw ODBIncomplete(dh.title(), Here());
    }

    return newTable;
}

//----------------------------------------------------------------------------------------------------------------------

}
}
