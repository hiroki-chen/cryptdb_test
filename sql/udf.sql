CREATE PROCEDURE clean_udf()
BEGIN
    DROP FUNCTION FHInsert;
    DROP FUNCTION FHUpdate;
    DROP FUNCTION FHSearch;
    DROP FUNCTION FHEnd;
    DROP FUNCTION FHStart;

    CREATE FUNCTION FHInsert RETURNS REAL SONAME 'ope.so';
    CREATE FUNCTION FHUpdate RETURNS REAL SONAME 'ope.so';
    CREATE FUNCTION FHSearch RETURNS REAL SONAME 'ope.so';
    CREATE FUNCTION FHEnd RETURNS REAL SONAME 'ope.so';
    CREATE FUNCTION FHStart RETURNS REAL SONAME 'ope.so';
END

CREATE PROCEDURE pro_insert(IN ciphertext varchar(128), IN pos INT, IN table_name VARCHAR(128))
BEGIN
    DECLARE cd REAL DEFAULT 0.0;
    SET cd = FHInsert(pos, ciphertext); -- Get the encoding.

    SET @sql = CONCAT_WS(" ", "INSERT INTO", table_name, "VALUES('", ciphertext, "',", cd , ")");
    PREPARE ex FROM @sql;
    EXECUTE ex;

    IF cd < 1 THEN
        SET @sql = CONCAT_WS(" ", "UPDATE", table_name, "SET encoding = FHUpdate(ciphertext) WHERE (encoding > FHStart() AND encoding < FHEnd()) OR (encoding = -1)");
        PREPARE ex FROM @sql;
        EXECUTE ex;
    END IF;
END //
DELIMITER ;