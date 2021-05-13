DROP PROCEDURE IF EXISTS update_on_video;
DELIMITER //

CREATE PROCEDURE update_on_video(IN video_id INT, IN cond INT)
BEGIN
    IF EXISTS (SELECT * FROM video WHERE video.id = video_id) THEN -- duplicate
        SET @msg = CONCAT("Update Error: duplicate id in video: ", video_id);
        SIGNAL SQLSTATE '45000' SET message_text = @msg;
    ELSE
        SET FOREIGN_KEY_CHECKS = 0;
        START TRANSACTION;
            SET @sql = CONCAT('UPDATE video SET id = ', video_id, ' WHERE video.id = ', cond);
            PREPARE ex FROM @sql;
            EXECUTE ex;
            
            SET @sql = CONCAT('UPDATE video_like SET video_id = ', video_id, ' WHERE video_like.video_id = ', cond);
            PREPARE ex FROM @sql;
            EXECUTE ex;
        COMMIT;
        SET FOREIGN_KEY_CHECKS = 1;

        SET @msg = CONCAT("Warning: You updated video id which may cause some problems: ", video_id);
        SIGNAL SQLSTATE '10000' SET message_text = @msg;
    END IF;
END //

DELIMITER ;