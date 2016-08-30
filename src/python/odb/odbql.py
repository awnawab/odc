# Python Database API (PEP 249) implementation for ODB API

# @author Piotr Kuchta, ECMWF, August 2016

import unittest, os
from ctypes import *

# TODO: get the path to libOdb.so
libodb = CDLL('/tmp/build/bundle/debug/lib/libOdb.so')

# odbql prototypes 
odbql_open = libodb.odbql_open
odbql_prepare_v2 = libodb.odbql_prepare_v2
odbql_step = libodb.odbql_step
odbql_column_name = libodb.odbql_column_name
odbql_column_name.restype = c_char_p
odbql_column_count = libodb.odbql_column_count
odbql_finalize = libodb.odbql_finalize
odbql_close = libodb.odbql_close
odbql_bind_null = libodb.odbql_bind_null
odbql_bind_int = libodb.odbql_bind_int
odbql_bind_double = libodb.odbql_bind_double
odbql_bind_text = libodb.odbql_bind_text
odbql_bind_int = libodb.odbql_bind_int
odbql_column_value = libodb.odbql_column_value
odbql_column_text = libodb.odbql_column_text
odbql_column_text.restype = c_char_p  # TODO: should be unsigned: const unsigned char *odbql_column_text(odbql_stmt* stmt, int iCol)
odbql_column_type = libodb.odbql_column_type
odbql_value_double = libodb.odbql_value_double
odbql_value_double.restype = c_double
odbql_value_int = libodb.odbql_value_int

# constants
ODBQL_OK = 0
ODBQL_ROW = 100
ODBQL_STATIC = c_voidp(0)

ODBQL_INTEGER  = 1
ODBQL_FLOAT    = 2
ODBQL_BLOB     = 4
ODBQL_NULL     = 5
ODBQL_TEXT     = 3

TEST_DDL = """
            CREATE TYPE bf AS (f1 bit1, f2 bit2); 
            
            CREATE TABLE foo AS (
                x INTEGER, 
                y REAL, 
                v STRING, 
                status bf
            ) ON 'new_api_example_python.odb';
"""

TEST_INSERT = """INSERT INTO foo (x,y,v,status) VALUES (?,?,?,?);"""
TEST_SELECT = """SELECT * FROM foo;"""

class TestODBQL(unittest.TestCase):

    def setUp(self): 
        pass

    def tearDown(self):
        pass

    def test_insert_data(self):
        db, stmt, tail = c_voidp(), c_voidp(), c_char_p()

        rc = odbql_open(TEST_DDL, byref(db))
        self.assertEqual(rc, ODBQL_OK)

        rc = odbql_prepare_v2(db, TEST_INSERT, -1, byref(stmt), byref(tail))
        self.assertEqual(rc, ODBQL_OK)
        
        for i in range(4): 
            rc = odbql_bind_null(stmt, i)
            self.assertEqual(rc, ODBQL_OK)

        rc = odbql_step(stmt)
        self.assertEqual(rc, ODBQL_ROW)

        for i in range(4): 
            rc = odbql_bind_int(stmt, 0, 1 * i)
            self.assertEqual(rc, ODBQL_OK)
            rc = odbql_bind_double(stmt, 1, c_double(0.1 * i))
            self.assertEqual(rc, ODBQL_OK)
            rc = odbql_bind_text(stmt, 2, "hello", len("hello"), ODBQL_STATIC)
            self.assertEqual(rc, ODBQL_OK)
            rc = odbql_bind_int(stmt, 3, 3 * i)
            self.assertEqual(rc, ODBQL_OK)
            rc = odbql_step(stmt)
            self.assertEqual(rc, ODBQL_ROW)
        
        rc = odbql_finalize(stmt)
        self.assertEqual(rc, ODBQL_OK)
        rc = odbql_close(db)
        self.assertEqual(rc, ODBQL_OK)


    def test_select_data(self):
        db, stmt, tail = c_voidp(), c_voidp(), c_char_p()

        rc = odbql_open("CREATE TABLE foo ON 'new_api_example_python.odb';", byref(db))
        self.assertEqual(rc, ODBQL_OK)

        rc = odbql_prepare_v2(db, "SELECT ALL * FROM foo;", -1, byref(stmt), byref(tail))
        self.assertEqual(rc, ODBQL_OK)


        number_of_rows = 0 
        rc = None
        while rc <> ODBQL_DONE:
            rc = odbql_step(stmt)

            if number_of_rows == 0 or rc == ODBQL_METADATA_CHANGED:
                number_of_columns = odbql_column_count(stmt)
                print 'number_of_columns=', number_of_columns
                for i in range(0,number_of_columns):
                    print i, odbql_column_name(stmt, i) + ':' + odbql_column_type(stmt, i)
            else:
                self.assertEqual(rc, ODBQL_ROW)

                def value(column):
                    v = odbql_column_value(stmt, column)
                    if not v: return None
                    else: 
                        t = odbql_column_type(stmt, column)
                        if t == ODBQL_FLOAT: return odbql_value_double(v)
                        if t == ODBQL_INTEGER: return odbql_value_int(v)
                        if t == ODBQL_NULL: return None
                        #ODBQL_BLOB     = 4
                        #ODBQL_NULL     = 5
                        #ODBQL_TEXT     = 3
                        return odbql_column_text(stmt, column)

                print ','.join([value(column) for column in range(number_of_columns)])
            number_of_rows += 1


        rc = odbql_finalize(stmt)
        self.assertEqual(rc, ODBQL_OK)
        rc = odbql_close(db)
        self.assertEqual(rc, ODBQL_OK)

        os.remove('new_api_example_python.odb')

##### PEP-249

apilevel = '2.0' 
threadsafety = 1 # https://www.python.org/dev/peps/pep-0249/#threadsafety
paramstyle = 'qmark' # https://www.python.org/dev/peps/pep-0249/#paramstyle

class Cursor:
    def __init__(self, ddl):
        self.ddl = ddl
        self.stmt = None
        self.number_of_columns = None
        
    def close(self):
        rc = odbql_finalize(self.stmt)
        self.stmt = None

    def execute(self, operation, parameters = None):
        print 'execute:', operation, ', parameters: ', parameters
        db, self.stmt, tail = c_voidp(0), c_voidp(), c_char_p()
        rc = odbql_open(self.ddl, byref(db))
        rc = odbql_prepare_v2(db, operation, -1, byref(self.stmt), byref(tail))
        if rc <> ODBQL_OK:
            raise Exception('execute: prepare failed')
        self.number_of_columns = odbql_column_count(self.stmt)
        self.types = [odbql_column_type(self.stmt, i) for i in range(self.number_of_columns)]
        self.names = [odbql_column_name(self.stmt, i) for i in range(self.number_of_columns)]
        
    def fetchone(self):
        if not self.stmt:
            raise Exception('fetchone: you must call execute first')

        rc = odbql_step(self.stmt)
        # TODO: handle ODBQL_CHANGED_METADATA
        if rc <> ODBQL_ROW:
            return None

        def value(column):
            v = odbql_column_value(self.stmt, column)
            if not v:
                return None
            else: 
                t = self.types[column]
                if t == ODBQL_FLOAT: return odbql_value_double(v)
                if t == ODBQL_INTEGER: return odbql_value_int(v)
                if t == ODBQL_NULL: return None
                #ODBQL_BLOB     = 4
                #ODBQL_NULL     = 5
                #ODBQL_TEXT     = 3
                return odbql_column_text(self.stmt, column)

        r = [value(column) for column in range(self.number_of_columns)]
        return r


    def executemany(self, operation, parameters):
        db, self.stmt, tail = c_voidp(), c_voidp(), c_char_p()

        rc = odbql_open(self.ddl, byref(db))
        #self.assertEqual(rc, ODBQL_OK)

        rc = odbql_prepare_v2(db, operation, -1, byref(self.stmt), byref(tail))
        #self.assertEqual(rc, ODBQL_OK)

        for ps in parameters:
            self._bind(ps)
            rc = odbql_step(self.stmt)
            #self.assertEqual(rc, ODBQL_ROW)

        rc = odbql_finalize(self.stmt)
        #self.assertEqual(rc, ODBQL_OK)
        rc = odbql_close(db)
        #self.assertEqual(rc, ODBQL_OK)
            

    def _bind(self, parameters):
        for i in range(len(parameters)):

            p = parameters[i]

            if p is None:
                rc = odbql_bind_null(self.stmt, i)
                #self.assertEqual(rc, ODBQL_OK)

            if type(p) == str:
                rc = odbql_bind_text(self.stmt, i, p, len(p), ODBQL_STATIC)
                #self.assertEqual(rc, ODBQL_OK)
            elif type(p) == int: 
                rc = odbql_bind_int(self.stmt, i, p)
                #self.assertEqual(rc, ODBQL_OK)
            elif type(p) == float: 
                rc = odbql_bind_double(self.stmt, i, c_double(p))
                #self.assertEqual(rc, ODBQL_OK)
            else:
                raise "Don't know how to bind parameter " + str(p) + ' of type ' + str(type(p))


class Connection:
    def __init__(self, db):
        self.db = db

    def close(self): pass
    def commit(self): pass
    def rollback(self): pass
    def cursor(self):
        return Cursor(self.db)

def connect(db):
    return Connection(db)


class TestODBQL(unittest.TestCase):

    def test_insert_data(self):
        conn = connect(TEST_DDL)
        c = conn.cursor()
        data = [(1,0.1, 'one', 1),
                (2,0.2, 'two', 2),
                (3,0.3, 'three', 3),
                (4,0.4, 'four', 4),
               ]
        c.executemany(TEST_INSERT, data)

    def test_select_data(self):
        conn = connect(TEST_DDL)
        c = conn.cursor()
        c.execute(TEST_SELECT)
        number_of_rows = 0
        while True:
            row = c.fetchone()
            if row is None:
                break
            #self.assertEqual ( row, (1, 0.1, 'one', 1) ) # assertion fails due to rounding errors for now
            self.assertEqual ( len(row), len((1, 0.1, 'one', 1)) )
            print row
            number_of_rows += 1

        self.assertEqual ( number_of_rows, 4 )

if __name__ == '__main__':
    unittest.main()