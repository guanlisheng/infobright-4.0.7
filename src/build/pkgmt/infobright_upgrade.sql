DELIMITER |
DROP PROCEDURE IF EXISTS `sys_infobright`.`CREATE_RULE`|
CREATE DEFINER=`root`@`localhost` PROCEDURE `sys_infobright`.`CREATE_RULE` ( IN id_ VARCHAR(4000), IN expression_ VARCHAR(4000), IN comment_ VARCHAR(4000) )
BEGIN
INSERT INTO sys_infobright.decomposition_dictionary ( ID, RULE, COMMENT ) VALUES( id_, expression_, comment_ );
END|

DROP PROCEDURE IF EXISTS `sys_infobright`.`UPDATE_RULE`|
CREATE DEFINER=`root`@`localhost` PROCEDURE `sys_infobright`.`UPDATE_RULE` ( IN id_ VARCHAR(4000), IN expression_ VARCHAR(4000) )
BEGIN
UPDATE sys_infobright.decomposition_dictionary SET RULE = expression_ WHERE ID = id_;
END|

DROP PROCEDURE IF EXISTS `sys_infobright`.`CHANGE_RULE_COMMENT`|
CREATE DEFINER=`root`@`localhost` PROCEDURE `sys_infobright`.`CHANGE_RULE_COMMENT` ( IN id_ VARCHAR(4000), IN comment_ VARCHAR(4000) )
BEGIN
UPDATE sys_infobright.decomposition_dictionary SET COMMENT = comment_ WHERE ID = id_;
END|

DROP PROCEDURE IF EXISTS `sys_infobright`.`SET_DECOMPOSITION_RULE`|
CREATE DEFINER=`root`@`localhost` PROCEDURE `sys_infobright`.`SET_DECOMPOSITION_RULE` ( IN database_ VARCHAR(4000), IN table_ VARCHAR(4000), IN column_ VARCHAR(4000), IN id_ VARCHAR(4000) )
BEGIN
DELETE FROM sys_infobright.columns WHERE database_name = database_ AND table_name = table_ AND column_name = column_;
INSERT INTO sys_infobright.columns ( database_name, table_name, column_name, decomposition ) VALUES( database_, table_, column_, id_ );
END|

DROP PROCEDURE IF EXISTS `sys_infobright`.`DELETE_DECOMPOSITION_RULE`|
CREATE DEFINER=`root`@`localhost` PROCEDURE `sys_infobright`.`DELETE_DECOMPOSITION_RULE` ( IN database_ VARCHAR(4000), IN table_ VARCHAR(4000), IN column_ VARCHAR(4000) )
BEGIN
DELETE FROM sys_infobright.columns WHERE database_name = database_ AND table_name = table_ AND column_name = column_;
END|

DROP PROCEDURE IF EXISTS `sys_infobright`.`SHOW_DECOMPOSITION`|
CREATE DEFINER=`root`@`localhost` PROCEDURE `sys_infobright`.`SHOW_DECOMPOSITION` ( IN database_ VARCHAR(4000), IN table_ VARCHAR(4000) )
BEGIN
SELECT s.ordinal_position AS '#', s.column_name AS 'column name', s.data_type AS 'data type', MIN( c.decomposition ) AS decomposition, MIN( d.rule ) AS rule FROM information_schema.columns AS s LEFT JOIN sys_infobright.columns c ON s.table_schema = c.database_name AND s.table_name = c.table_name AND s.column_name = c.column_name LEFT JOIN sys_infobright.decomposition_dictionary d ON c.decomposition = d.id WHERE s.table_schema = database_ AND s.table_name = table_ GROUP BY s.ordinal_position, s.column_name, s.data_type ORDER BY s.ordinal_position;
END|
