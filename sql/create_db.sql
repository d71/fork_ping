CREATE DATABASE IF NOT EXISTS `ping` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;
USE `ping`;


CREATE TABLE IF NOT EXISTS `hosts` (
  `st` int(11) DEFAULT NULL,
  `ip` varchar(15) DEFAULT NULL,
  `life` int(11) NOT NULL,
  `description` varchar(250) DEFAULT NULL,
  `pos` varchar(50) NOT NULL,
  `sub_ip` varchar(50) DEFAULT NULL,
  `who` varchar(50) NOT NULL,
  `lan_group` int(11) NOT NULL,
  `gsm_num` varchar(15) NOT NULL
);

CREATE TABLE IF NOT EXISTS `timeline` (
  `host` varchar(50) NOT NULL,
  `ping` int(11) NOT NULL,
  `time1` datetime NOT NULL
);
