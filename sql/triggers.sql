DROP TRIGGER IF EXISTS insert_before_video_like;
DELIMITER //

CREATE TRIGGER IF NOT EXISTS insert_before_video_like BEFORE INSERT ON video_like
FOR EACH ROW
BEGIN
    IF (NEW.video_id NOT IN (SELECT id FROM video)) THEN
        SET @msg = CONCAT('\tMyTriggerError: Trying to insert non-existing value in video_like! video_id: ', NEW.video_id);
        SIGNAL SQLSTATE '45000' SET message_text = @msg;
    ELSE
        IF (NEW.user_id NOT IN (SELECT id FROM user)) THEN
            SET @msg = CONCAT('\tMyTriggerError: Trying to insert non-existing value in video_like! user_id: ', NEW.user_id);
            SIGNAL SQLSTATE '45000' SET message_text = @msg;
        END IF;
     END IF;
END //

DELIMITER ;