/*

export ORACLE_SID=daetoy
export ORACLE_HOME=/u01/app/oracle/product/11.2.0/dbhome_1

/u01/app/oracle/product/11.2.0/dbhome_1/bin/expdp expimp/expimp directory=exp_dmp owner=dezhao dumpfile=schema_expdp.dmp rows=y consistent=y

*/

create or replace directory exp_dmp as '/dae/backups/daily/oracle/';
CREATE USER expimp IDENTIFIED BY <password> default tablespace users;

GRANT CONNECT,RESOURCE TO expimp;
GRANT exp_full_database to expimp;
GRANT imp_full_database to expimp;

GRANT READ, WRITE ON DIRECTORY exp_dmp to expimp;