CREATE TABLE IF NOT EXISTS `game`.`users` (
  `UID` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
  `UserID` VARCHAR(64) NOT NULL,
  `PassWord` VARCHAR(64) NOT NULL,
  PRIMARY KEY (`UID`),
  UNIQUE INDEX `UserID_UNIQUE` (`UserID` ASC))
ENGINE = InnoDB
AUTO_INCREMENT = 24
DEFAULT CHARACTER SET = utf8



DELIMITER $$
USE `game`$$
CREATE DEFINER=`root`@`%` PROCEDURE `CheckLoginData`(IN _UserID VARCHAR(64), IN _PassWord VARCHAR(64), OUT result TINYINT)
BEGIN
    SELECT EXISTS(
		SELECT * 
		FROM users 
        WHERE 
			UserID = _UserID AND 
            PassWord = _PassWord
	) INTO result;
    
    #SET result = 12;
    SELECT result;
END$$

DELIMITER ;



DELIMITER $$
USE `game`$$
CREATE DEFINER=`root`@`%` PROCEDURE `CreateUser`(IN _UserID VARCHAR(64), IN _PassWord VARCHAR(64))
BEGIN
	INSERT INTO users(UserID, PassWord) VALUES(_UserID, _PassWord);
END$$

DELIMITER ;


DELIMITER $$
USE `game`$$
CREATE DEFINER=`root`@`%` PROCEDURE `DeleteUser`(IN _UserID VARCHAR(64))
BEGIN
	DELETE FROM users WHERE UserID = _UserID;
END$$

DELIMITER ;


