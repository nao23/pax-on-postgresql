pax-on-postgresql
=================

このプログラムは情報特別演習2という授業で作成したものです。
PAXというページレイアウトモデルをPostgreSQLに実装することを目標にしていますが、
まだ限定的な条件でないと実行できません。

## 対象とするテーブル
今回対象とするテーブルはinteger型のデータのみで構成されるテーブルです。
カラム数が大きくなるとエラーが発生してしまうため32x32程度でないとできません。
tools/nsm_to_pax.cでPAXレイアウトのページに変換する。

## 対象とするクエリ
select id from test where a1 = 100 and a2 = 200;

のような、where句の条件式がandで結ばれていて、属性と値の比較演算子が = になっているもの。

## 実装
今回利用したPostgreSQLのバージョンは9.3.5です。
具体的には以下の部分を変更しました。

### postgresql-9.3.5/src/backend/executor/execMain.c
性能計測のためPAPIを利用するので、standard_ExecutorRun内にPAPIのコードを追加

### postgresql-9.3.5/src/include/relscan.h
PAXレイアウト用に、HeapScanDescData構造体にメンバを追加

### postgresql-9.3.5/src/backend/executor/nodeSeqScan.c
Where句の条件式を取得して、HeapScanDescData構造体に保存する関数create_paxscan_argsを追加

SeqNext内で、heap_getnextを呼ぶ前にcreate_paxscan_argsを呼ぶように変更

### postgresql-9.3.5/src/backend/access/heap/heapam.c
以下の関数を変更
* initscan 
  - HeapScanDescData構造体の初期化を行うので、新たに追加したメンバに対する初期化処理を追加
* heap_getnext 
  - NSMレイアウトのテーブルならheapgettup_pagemodeを、PAXレイアウトのテーブルなら
pax_heapgettup_pagemodeを呼ぶように変更
* heapgettup_pagemode 
  - PAXとの比較のために、tuple->t_dataをmalloc & memcpyするように変更

以下の関数を新たに追加
* pax_heapgettup_pagemode(HeapScanDesc scan, ScanDirection dir, int nkeys, ScanKey key);
  - PAXレイアウトのテーブルに対して、スキャンを行い、その結果をもとにタプルを返す関数
* scan_paxpage(HeapScanDesc scan, Page pax_dp);
  - PAXレイアウトのページをスキャンする関数
* construct_nsmtuple(Page pax_dp, int nsm_ncols, HeapTupleHeader t_header, OffsetNumber lineoff);
  - スキャンした結果から、NSMタプルを作成する関数

