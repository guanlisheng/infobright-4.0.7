--
-- The inital data for system tables of MySQL Server
--

-- When setting up a "cross bootstrap" database (e.g., creating data on a Unix
-- host which will later be included in a Windows zip file), any lines
-- containing "@current_hostname" are filtered out by mysql_install_db.
set @current_hostname= @@hostname;


-- Fill "db" table with default grants for anyone to
-- access database 'test' and 'test_%' if "db" table didn't exist
CREATE TEMPORARY TABLE tmp_db LIKE db;
INSERT INTO tmp_db VALUES ('%','test','','Y','Y','Y','Y','Y','Y','N','Y','Y','Y','Y','Y','Y','Y','Y','N','N','Y','Y');
INSERT INTO tmp_db VALUES ('%','test\_%','','Y','Y','Y','Y','Y','Y','N','Y','Y','Y','Y','Y','Y','Y','Y','N','N','Y','Y');
INSERT INTO db SELECT * FROM tmp_db WHERE @had_db_table=0;
DROP TABLE tmp_db;


-- Fill "users" table with default users allowing root access
-- from local machine if "users" table didn't exist before
CREATE TEMPORARY TABLE tmp_user LIKE user;
set @current_hostname= @@hostname;
INSERT INTO tmp_user VALUES ('localhost','root','','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','','','','',0,0,0,0);
REPLACE INTO tmp_user SELECT @current_hostname,'root','','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','','','','',0,0,0,0 FROM dual WHERE LOWER( @current_hostname) != 'localhost';
REPLACE INTO tmp_user VALUES ('127.0.0.1','root','','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','Y','','','','',0,0,0,0);
INSERT INTO tmp_user (host,user) VALUES ('localhost','');
INSERT INTO tmp_user (host,user) SELECT @current_hostname,'' FROM dual WHERE LOWER(@current_hostname ) != 'localhost';
INSERT INTO user SELECT * FROM tmp_user WHERE @had_user_table=0;
DROP TABLE tmp_user;

CREATE TEMPORARY TABLE tmp_decomp LIKE sys_infobright.decomposition_dictionary;
INSERT INTO tmp_decomp VALUES ('IPv4','PREDEFINED','');
INSERT INTO tmp_decomp VALUES ('IPv4_C','%d.%d.%d.%d','');
INSERT INTO tmp_decomp VALUES ('EMAIL','%s@%s','');
INSERT INTO tmp_decomp VALUES ('URL','%s://%s?%s','');
INSERT INTO sys_infobright.decomposition_dictionary SELECT * FROM tmp_decomp WHERE @had_decomp_dict_table=0;
DROP TABLE tmp_decomp;

DROP trigger IF EXISTS sys_infobright.before_insert_on_columns; 
CREATE DEFINER=`root`@`localhost` TRIGGER sys_infobright.before_insert_on_columns BEFORE INSERT ON sys_infobright.columns FOR EACH ROW BEGIN select count(*) from sys_infobright.columns where database_name = NEW.database_name and table_name = NEW.table_name and column_name = NEW.column_name into @count; IF @count <> 0 THEN SET @@brighthouse_trigger_error = concat ('Insert failed: The decomposition for column ', NEW.database_name, '.', NEW.table_name, '.', NEW.column_name, ' already set.'); END IF; select count(*) from sys_infobright.decomposition_dictionary where ID = NEW.decomposition into @count; IF @count = 0 THEN SET @@brighthouse_trigger_error = concat ('Insert failed: The decomposition \'', NEW.decomposition, '\' does not exist.'); END IF; END ;

DROP trigger IF EXISTS sys_infobright.before_update_on_columns;
CREATE DEFINER=`root`@`localhost` TRIGGER sys_infobright.before_update_on_columns BEFORE UPDATE ON sys_infobright.columns FOR EACH ROW BEGIN SELECT COUNT(*) FROM sys_infobright.decomposition_dictionary WHERE ID = NEW.decomposition INTO @count; IF @count = 0 THEN SET @@brighthouse_trigger_error = concat ('Update failed: The decomposition \'', NEW.decomposition, '\' does NOT exist.'); END IF; END ;

DROP trigger IF EXISTS sys_infobright.before_insert_on_decomposition_dictionary;
CREATE DEFINER=`root`@`localhost` TRIGGER sys_infobright.before_insert_on_decomposition_dictionary BEFORE INSERT ON sys_infobright.decomposition_dictionary FOR EACH ROW BEGIN SELECT COUNT(*) FROM sys_infobright.decomposition_dictionary WHERE ID = NEW.ID INTO @count; IF @count <> 0 THEN SET @@brighthouse_trigger_error = concat ('Insert failed: The decomposition \'', NEW.ID, '\' already exists.'); END IF; select is_decomposition_rule_correct( NEW.RULE ) into @valid; IF @valid = 0 THEN SET @@brighthouse_trigger_error = concat ('Insert failed: The decomposition rule \'', NEW.RULE, '\' is invalid.'); END IF; END ;

DROP trigger IF EXISTS sys_infobright.before_update_on_decomposition_dictionary; 
CREATE DEFINER=`root`@`localhost` TRIGGER sys_infobright.before_update_on_decomposition_dictionary BEFORE UPDATE ON decomposition_dictionary FOR EACH ROW BEGIN IF OLD.RULE <> NEW.RULE then select is_decomposition_rule_correct( NEW.RULE ) into @valid; IF @valid = 0 THEN SET @@brighthouse_trigger_error = concat ('Update failed: The decomposition rule \'', NEW.RULE, '\' is invalid.'); END IF; END IF; IF OLD.ID <> NEW.ID then SET @@brighthouse_trigger_error = concat ('Update failed: The column ID is not allowed to be changed in DECOMPOSITION_DICTIONARY.'); END IF; END ;

DROP trigger IF EXISTS sys_infobright.before_delete_on_decomposition_dictionary;
CREATE DEFINER=`root`@`localhost` TRIGGER sys_infobright.before_delete_on_decomposition_dictionary BEFORE DELETE ON sys_infobright.decomposition_dictionary FOR EACH ROW BEGIN SELECT COUNT(*) FROM sys_infobright.columns WHERE decomposition = OLD.ID INTO @count; IF @count <> 0 THEN SET @@brighthouse_trigger_error = concat ('Delete failed: The decomposition \'', OLD.ID, '\' IS used IN at least one column.'); END IF; END ;

DROP trigger IF EXISTS sys_infobright.after_insert_on_columns;
CREATE DEFINER=`root`@`localhost` TRIGGER sys_infobright.after_insert_on_columns AFTER INSERT ON sys_infobright.columns FOR EACH ROW BEGIN SET @@global.brighthouse_refresh_sys_infobright = TRUE; END ;

DROP trigger IF EXISTS sys_infobright.after_update_on_columns;
CREATE DEFINER=`root`@`localhost` TRIGGER sys_infobright.after_update_on_columns AFTER UPDATE ON sys_infobright.columns FOR EACH ROW BEGIN SET @@global.brighthouse_refresh_sys_infobright = TRUE; END ;

DROP trigger IF EXISTS sys_infobright.after_delete_on_columns;
CREATE DEFINER=`root`@`localhost` TRIGGER sys_infobright.after_delete_on_columns AFTER DELETE ON sys_infobright.columns FOR EACH ROW BEGIN SET @@global.brighthouse_refresh_sys_infobright = TRUE; END ;

DROP trigger IF EXISTS sys_infobright.after_insert_on_decomposition_dictionary;
CREATE DEFINER=`root`@`localhost` TRIGGER sys_infobright.after_insert_on_decomposition_dictionary AFTER INSERT ON sys_infobright.decomposition_dictionary FOR EACH ROW BEGIN SET @@global.brighthouse_refresh_sys_infobright = TRUE; END ;

DROP trigger IF EXISTS sys_infobright.after_update_on_decomposition_dictionary;
CREATE DEFINER=`root`@`localhost` TRIGGER sys_infobright.after_update_on_decomposition_dictionary AFTER UPDATE ON sys_infobright.decomposition_dictionary FOR EACH ROW BEGIN SET @@global.brighthouse_refresh_sys_infobright = TRUE; END ;

DROP trigger IF EXISTS sys_infobright.after_delete_on_decomposition_dictionary;
CREATE DEFINER=`root`@`localhost` TRIGGER sys_infobright.after_delete_on_decomposition_dictionary AFTER DELETE ON sys_infobright.decomposition_dictionary FOR EACH ROW BEGIN  SET @@global.brighthouse_refresh_sys_infobright = TRUE; END ;

DROP PROCEDURE IF EXISTS `sys_infobright`.`CREATE_RULE`;
CREATE DEFINER=`root`@`localhost` PROCEDURE `sys_infobright`.`CREATE_RULE` ( IN id_ VARCHAR(4000), IN expression_ VARCHAR(4000), IN comment_ VARCHAR(4000) ) BEGIN INSERT INTO sys_infobright.decomposition_dictionary ( ID, RULE, COMMENT ) VALUES( id_, expression_, comment_ ); END;

DROP PROCEDURE IF EXISTS `sys_infobright`.`UPDATE_RULE`;
CREATE DEFINER=`root`@`localhost` PROCEDURE `sys_infobright`.`UPDATE_RULE` ( IN id_ VARCHAR(4000), IN expression_ VARCHAR(4000) ) BEGIN UPDATE sys_infobright.decomposition_dictionary SET RULE = expression_ WHERE ID = id_; END;

DROP PROCEDURE IF EXISTS `sys_infobright`.`DELETE_RULE`;
CREATE DEFINER=`root`@`localhost` PROCEDURE `sys_infobright`.`DELETE_RULE` ( IN id_ VARCHAR(4000) ) BEGIN DELETE FROM sys_infobright.decomposition_dictionary WHERE ID = id_; END;

DROP PROCEDURE IF EXISTS `sys_infobright`.`CHANGE_RULE_COMMENT`;
CREATE DEFINER=`root`@`localhost` PROCEDURE `sys_infobright`.`CHANGE_RULE_COMMENT` ( IN id_ VARCHAR(4000), IN comment_ VARCHAR(4000) ) BEGIN UPDATE sys_infobright.decomposition_dictionary SET COMMENT = comment_ WHERE ID = id_; END;

DROP PROCEDURE IF EXISTS `sys_infobright`.`SET_DECOMPOSITION_RULE`;
CREATE DEFINER=`root`@`localhost` PROCEDURE `sys_infobright`.`SET_DECOMPOSITION_RULE` ( IN database_ VARCHAR(4000), IN table_ VARCHAR(4000), IN column_ VARCHAR(4000), IN id_ VARCHAR(4000) ) BEGIN DELETE FROM sys_infobright.columns WHERE database_name = database_ AND table_name = table_ AND column_name = column_; INSERT INTO sys_infobright.columns ( database_name, table_name, column_name, decomposition ) VALUES( database_, table_, column_, id_ ); END;

DROP PROCEDURE IF EXISTS `sys_infobright`.`DELETE_DECOMPOSITION_RULE`;
CREATE DEFINER=`root`@`localhost` PROCEDURE `sys_infobright`.`DELETE_DECOMPOSITION_RULE` ( IN database_ VARCHAR(4000), IN table_ VARCHAR(4000), IN column_ VARCHAR(4000) ) BEGIN DELETE FROM sys_infobright.columns WHERE database_name = database_ AND table_name = table_ AND column_name = column_; END;

DROP PROCEDURE IF EXISTS `sys_infobright`.`SHOW_DECOMPOSITION`;
CREATE DEFINER=`root`@`localhost` PROCEDURE `sys_infobright`.`SHOW_DECOMPOSITION` ( IN database_ VARCHAR(4000), IN table_ VARCHAR(4000) ) BEGIN SELECT s.ordinal_position AS '#', s.column_name AS 'column name', s.data_type AS 'data type', MIN( c.decomposition ) AS decomposition, MIN( d.rule ) AS rule FROM information_schema.columns AS s LEFT JOIN sys_infobright.columns c ON s.table_schema = c.database_name AND s.table_name = c.table_name AND s.column_name = c.column_name LEFT JOIN sys_infobright.decomposition_dictionary d ON c.decomposition = d.id WHERE s.table_schema = database_ AND s.table_name = table_ GROUP BY s.ordinal_position, s.column_name, s.data_type ORDER BY s.ordinal_position; END;

