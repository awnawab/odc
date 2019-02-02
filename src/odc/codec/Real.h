/*
 * (C) Copyright 1996-2012 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef odc_core_codec_Real_H
#define odc_core_codec_Real_H

#include "odc/core/Codec.h"


namespace odc {
namespace codec {

//----------------------------------------------------------------------------------------------------------------------

static constexpr uint32_t minFloatAsInt = 0x800000;
static constexpr uint32_t maxFloatAsInt = 0x7f7fffff;

//----------------------------------------------------------------------------------------------------------------------

template<typename ByteOrder>
class CodecLongReal : public core::DataStreamCodec<ByteOrder> {

public: // definitions

    constexpr static const char* codec_name() { return "long_real"; }

public: // methods

    CodecLongReal() :
        core::DataStreamCodec<ByteOrder>(codec_name()),
        hasShortRealInternalMissing_(false),
        hasShortReal2InternalMissing_(false) {}

    ~CodecLongReal() override {}

    bool hasShortRealInternalMissing() const { return hasShortRealInternalMissing_; }
    bool hasShortReal2InternalMissing() const { return hasShortReal2InternalMissing_; }

private: // methods

    unsigned char* encode(unsigned char* p, const double& d) override {
        double e = d;
        ByteOrder::swap(e);
        memcpy(p, &e, sizeof(e));
        return p + sizeof(e);
    }

    void decode(double* out) override {
        this->ds().read(*out);
    }

    /// Keep track on internal missing value collisions, to help the CodecOptimizer.
    void gatherStats(const double& v) override {
        core::Codec::gatherStats(v);

        float realInternalMissing = reinterpret_cast<const float&>(minFloatAsInt);
        float realInternalMissing2 = reinterpret_cast<const float&>(maxFloatAsInt);
        if (v == realInternalMissing) hasShortRealInternalMissing_ = true;
        if (v == realInternalMissing2) hasShortReal2InternalMissing_ = true;
    }

private: // members

    bool hasShortRealInternalMissing_;
    bool hasShortReal2InternalMissing_;
};


//----------------------------------------------------------------------------------------------------------------------

template <typename ByteOrder, uint32_t InternalMissing>
class ShortRealBase : public core::DataStreamCodec<ByteOrder> {

public: // methods

    ShortRealBase(const std::string& name) : core::DataStreamCodec<ByteOrder>(name) {}
    ~ShortRealBase() override {}

private: // methods

    unsigned char* encode(unsigned char* p, const double& d) override {

        float s;
        if (d == this->missingValue_) {
            s = InternalMissing;
        } else {
            s = d;
            ASSERT(s != InternalMissing);
        }

        ByteOrder::swap(s);
        ::memcpy(p, &s, sizeof(s));
        return p + sizeof(s);
    }

    void decode(double* out) override {
        float s;
        this->ds().read(s);
        uint32_t internalMissingInt = InternalMissing;
        float internalMissing = reinterpret_cast<float&>(internalMissingInt);
        (*out) = (s == internalMissing ? this->missingValue_ : s);
    }
};


template <typename ByteOrder>
struct CodecShortReal : public ShortRealBase<ByteOrder, minFloatAsInt> {
    constexpr static const char* codec_name() { return "short_real"; }
    CodecShortReal() : ShortRealBase<ByteOrder, minFloatAsInt>(codec_name()) {}
    ~CodecShortReal() {}
};


template <typename ByteOrder>
struct CodecShortReal2 : public ShortRealBase<ByteOrder, maxFloatAsInt> {
    constexpr static const char* codec_name() { return "short_real2"; }
    CodecShortReal2() : ShortRealBase<ByteOrder, maxFloatAsInt>(codec_name()) {}
    ~CodecShortReal2() {}
};


//----------------------------------------------------------------------------------------------------------------------

} // namespace codec
} // namespace odc

#endif

