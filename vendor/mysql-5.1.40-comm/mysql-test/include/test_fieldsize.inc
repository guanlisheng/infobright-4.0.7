#
# include/test_fieldsize.inc 
# 
# This include file is designed to create a table with one column
# whose size on the master is greater than that on the slave. The
# test should fail with an error on the slave.
#

connection master;
DROP TABLE IF EXISTS t1;

sync_slave_with_master;
STOP SLAVE;
RESET SLAVE;
eval $test_table_slave;

connection master;
eval $test_table_master;
RESET MASTER;

eval $test_insert;

connection slave;
START SLAVE;
wait_for_slave_to_stop;
--replace_result $MASTER_MYPORT MASTER_PORT
--replace_column 1 # 4 # 7 # 8 # 9 # 16 # 22 # 23 # 33 # 35 # 36 #
--query_vertical SHOW SLAVE STATUS

# The following should be 0
SELECT COUNT(*) FROM t1;
STOP SLAVE;
RESET SLAVE;

connection master;
RESET MASTER;

connection slave;
START SLAVE;

