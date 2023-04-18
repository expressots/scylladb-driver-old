# type mapping

| Database type | Typescript type | Check | Comments        |
| ------------- | --------------- | ----- | --------------- |
| text          | string          | ✅    |                 |
| varchar       | string          | ✅    |                 |
| int           | int             | ✅    |                 |
| boolean       | boolean         | ✅    |                 |
| uuid          | string          | ✅    |                 |
| bigint        | bigint          | ✅    |                 |
| float         | number          | ✅    |                 |
| double        | number          | ✅    |                 |
| variant       | integer         | ⛔️   |                 |
| timestamp     | number          | ✅    | string / number |

---

```cpp
#define CASS_VALUE_TYPE_MAPPING(XX) \
 XX(CASS_VALUE_TYPE_CUSTOM, 0x0000, "", "") \
 XX(CASS_VALUE_TYPE_ASCII, 0x0001, "ascii", "org.apache.cassandra.db.marshal.AsciiType") \
 XX(CASS_VALUE_TYPE_BIGINT, 0x0002, "bigint", "org.apache.cassandra.db.marshal.LongType") \
 XX(CASS_VALUE_TYPE_BLOB, 0x0003, "blob", "org.apache.cassandra.db.marshal.BytesType") \
 XX(CASS_VALUE_TYPE_BOOLEAN, 0x0004, "boolean", "org.apache.cassandra.db.marshal.BooleanType") \
 XX(CASS_VALUE_TYPE_COUNTER, 0x0005, "counter", "org.apache.cassandra.db.marshal.CounterColumnType") \
 XX(CASS_VALUE_TYPE_DECIMAL, 0x0006, "decimal", "org.apache.cassandra.db.marshal.DecimalType") \
 XX(CASS_VALUE_TYPE_DOUBLE, 0x0007, "double", "org.apache.cassandra.db.marshal.DoubleType") \
 XX(CASS_VALUE_TYPE_FLOAT, 0x0008, "float", "org.apache.cassandra.db.marshal.FloatType") \
 XX(CASS_VALUE_TYPE_INT, 0x0009, "int", "org.apache.cassandra.db.marshal.Int32Type") \
 XX(CASS_VALUE_TYPE_TEXT, 0x000A, "text", "org.apache.cassandra.db.marshal.UTF8Type") \
 XX(CASS_VALUE_TYPE_TIMESTAMP, 0x000B, "timestamp", "org.apache.cassandra.db.marshal.TimestampType") \
 XX(CASS_VALUE_TYPE_UUID, 0x000C, "uuid", "org.apache.cassandra.db.marshal.UUIDType") \
 XX(CASS_VALUE_TYPE_VARCHAR, 0x000D, "varchar", "") \
 XX(CASS_VALUE_TYPE_VARINT, 0x000E, "varint", "org.apache.cassandra.db.marshal.IntegerType") \
 XX(CASS_VALUE_TYPE_TIMEUUID, 0x000F, "timeuuid", "org.apache.cassandra.db.marshal.TimeUUIDType") \
 XX(CASS_VALUE_TYPE_INET, 0x0010, "inet", "org.apache.cassandra.db.marshal.InetAddressType") \
 XX(CASS_VALUE_TYPE_DATE, 0x0011, "date", "org.apache.cassandra.db.marshal.SimpleDateType") \
 XX(CASS_VALUE_TYPE_TIME, 0x0012, "time", "org.apache.cassandra.db.marshal.TimeType") \
 XX(CASS_VALUE_TYPE_SMALL_INT, 0x0013, "smallint", "org.apache.cassandra.db.marshal.ShortType") \
 XX(CASS_VALUE_TYPE_TINY_INT, 0x0014, "tinyint", "org.apache.cassandra.db.marshal.ByteType") \
 XX(CASS_VALUE_TYPE_DURATION, 0x0015, "duration", "org.apache.cassandra.db.marshal.DurationType") \
 XX(CASS_VALUE_TYPE_LIST, 0x0020, "list", "org.apache.cassandra.db.marshal.ListType") \
 XX(CASS_VALUE_TYPE_MAP, 0x0021, "map", "org.apache.cassandra.db.marshal.MapType") \
 XX(CASS_VALUE_TYPE_SET, 0x0022, "set", "org.apache.cassandra.db.marshal.SetType") \
 XX(CASS_VALUE_TYPE_UDT, 0x0030, "", "") \
 XX(CASS_VALUE_TYPE_TUPLE, 0x0031, "tuple", "org.apache.cassandra.db.marshal.TupleType")
```
