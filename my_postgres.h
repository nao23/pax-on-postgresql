typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef signed int int32;
typedef uint32 TransactionId;
typedef uint16 LocationIndex;
typedef uint32 CommandId;
typedef uint8 bits8;
typedef unsigned int Oid;
typedef uint16 OffsetNumber;


#define BLCKSZ 8192

typedef struct ItemIdData
{
        unsigned lp_off:15, lp_flags:2, lp_len:15;
} ItemIdData;

typedef struct
{
        uint32 xlogid;
        uint32 xrecoff;
} PageXLogRecPtr;

typedef struct PageHeaderData
{
        PageXLogRecPtr pd_lsn;
        uint16 pd_checksum;
        uint16 pd_flags;
        LocationIndex pd_lower;
        LocationIndex pd_upper;
        LocationIndex pd_special;
        uint16 pd_pagesize_version;
        TransactionId pd_prune_xid;
        ItemIdData pd_linp[1];
} PageHeaderData;

typedef PageHeaderData *PageHeader;

typedef struct HeapTupleFields
{
        TransactionId t_xmin;
        TransactionId t_xmax;

	union
        {
                CommandId t_cid;
                TransactionId t_xvac;
        } t_fields3;

} HeapTupleFields;

typedef struct DataTupleFields
{
        int32 datum_len_;
        int32 datum_typmod;
        Oid datum_typeid;
} DatumTupleFields;

typedef struct BlockIdData
{
        uint16 bi_hi;
        uint16 bi_lo;
} BlockIdData;

typedef struct ItemPointerData
{
        BlockIdData ip_blkid;
        OffsetNumber ip_postid;
} ItemPointerData;


struct HeapTupleHeaderData
{
        union
        {
                HeapTupleFields t_heap;
                DatumTupleFields t_datum;
        } t_choise;

	ItemPointerData t_ctid;

	uint16 t_infomask2;
        uint16 t_infomask;
        uint8 t_hoff;

	bits8 t_bits[1];
};

typedef struct HeapTupleHeaderData HeapTupleHeaderData;

typedef HeapTupleHeaderData *HeapTupleHeader;

#define HEAP_NATTS_MASK                 0x07FF

#define HeapTupleHeaderGetNatts(tup)			\
        ((tup)->t_infomask2 & HEAP_NATTS_MASK)
 

#define HeapTupleHeaderSetNatts(tup, natts) \
( \
	(tup)->t_infomask2 = ((tup)->t_infomask2 & ~HEAP_NATTS_MASK) | (natts) \
)

