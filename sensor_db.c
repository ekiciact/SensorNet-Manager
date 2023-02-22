#include "sensor_db.h"

DBCONN *init_connection(char clear_up_flag) {
    DBCONN *conn;
    int rc;

    // create or open the database
    rc = sqlite3_open(TO_STRING(DB_NAME), &conn);
    if (rc) {
        // unable to connect to SQL server
        log_event("Unable to connect to SQL server.");
        return NULL;
    } else {
        log_event("Connected to SQL server.");
    }

    //create table
    char *create_table_query = "CREATE TABLE IF NOT EXISTS " TO_STRING(
            TABLE_NAME) " (id INTEGER PRIMARY KEY AUTOINCREMENT, sensor_id INT, sensor_value DECIMAL(4, 2), timestamp TIMESTAMP);";
    char *clear_table_query = "DELETE FROM " TO_STRING(TABLE_NAME) ";";
    rc = sqlite3_exec(conn, create_table_query, 0, 0, 0);
    if (rc != SQLITE_OK) {
        log_event("Error creating table.");
        return NULL;
    } else {
        log_event("Table created/opened.");
    }

    //clear up the table if the flag is set
    if (clear_up_flag) {
        rc = sqlite3_exec(conn, clear_table_query, 0, 0, 0);
        if (rc != SQLITE_OK) {
            log_event("Error clearing up table.");
        } else {
            log_event("Table cleared up.");
        }
    }

    return conn;
}

void storagemgr_parse_sensor_data(DBCONN *conn, sbuffer_t **buffer) {
    int conn_attempts = 0;
    while (*buffer) {
        if (conn == NULL) {
            if (conn_attempts == 3) {
                log_event("Unable to connect to SQL server.\n");
                exit(EXIT_FAILURE);
            }
            log_event("Connection to SQL server lost.\n");
            conn = init_connection(0);
            conn_attempts++;
            sleep(5);
            continue;
        }
        sensor_data_t sensor_data;
        int status = sbuffer_remove(*buffer, &sensor_data);
        if (status == SBUFFER_SUCCESS) {
            int insert_status = insert_sensor(conn, sensor_data.id, sensor_data.value, sensor_data.ts);
            if (insert_status != 0) {
                log_event("Data insertion failed.\n");
                conn_attempts++;
                continue;
            }
        }
        conn_attempts = 0;
    }
}

int insert_sensor(DBCONN *conn, sensor_id_t id, sensor_value_t value, sensor_ts_t ts) {
    int result_code;
    char sql[SQL_MAX_LEN];
    sqlite3_stmt *stmt;
    sprintf(sql, "INSERT INTO %s (sensor_id, sensor_value, timestamp) VALUES (?,?,?)", TO_STRING(TABLE_NAME));
    result_code = sqlite3_prepare_v2(conn, sql, -1, &stmt, NULL);
    if (result_code != SQLITE_OK) {
        log_event("Data insertion prepare error: %s\n", sqlite3_errmsg(conn));
        return result_code;
    }
    sqlite3_bind_int(stmt, 1, id);
    sqlite3_bind_double(stmt, 2, value);
    sqlite3_bind_int64(stmt, 3, ts);
    result_code = sqlite3_step(stmt);
    if (result_code != SQLITE_DONE) {
        log_event("Data insertion execution error: %s\n", sqlite3_errmsg(conn));
    }
    sqlite3_finalize(stmt);
    return result_code;
}

void disconnect(DBCONN *conn) {
    int ret = sqlite3_close(conn);
    if (ret != SQLITE_OK) {
        log_event("Error occured while disconnecting from the SQL server: %s\n", sqlite3_errmsg(conn));
    } else {
        log_event("Disconnected from the SQL server.\n");
    }
}

int find_sensor_all(DBCONN *conn, callback_t f) {
    int rc;
    char *err_msg = 0;
    char query[255];

    snprintf(query, sizeof query, "SELECT * FROM %s;", TO_STRING(TABLE_NAME));
    rc = sqlite3_exec(conn, query, f, 0, &err_msg);
    if (rc != SQLITE_OK) {
        log_event("SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return rc;
    }
    return rc;
}

int find_sensor_by_value(DBCONN *conn, sensor_value_t value, callback_t f) {
    char sql[200];
    snprintf(sql, sizeof(sql), "SELECT * FROM %s WHERE sensor_value = %lf", TABLE_NAME, value);
    return sqlite3_exec(conn, sql, f, 0, 0);
}

int find_sensor_exceed_value(DBCONN *conn, sensor_value_t value, callback_t f) {
// Prepare the SQL statement
    char sql[255];
    snprintf(sql, sizeof(sql), "SELECT * FROM %s WHERE sensor_value > %f", TABLE_NAME, value);
// Execute the statement
    return sqlite3_exec(conn, sql, f, 0, 0);
}

int find_sensor_by_timestamp(DBCONN *conn, sensor_ts_t ts, callback_t f) {
    char sql[150];
    int rc;
    sprintf(sql, "SELECT * FROM %s WHERE timestamp = %lu;", TABLE_NAME, ts);
    rc = sqlite3_exec(conn, sql, f, 0, 0);

    if (rc != SQLITE_OK) {
        log_event("Failed to select data by timestamp. Error: %s\n", sqlite3_errmsg(conn));
        return 1;
    }
    log_event("Selected data by timestamp successfully.\n");
    return 0;
}

int find_sensor_after_timestamp(DBCONN *conn, sensor_ts_t ts, callback_t f) {
    char *zErrMsg = 0;
    int rc;
    char sql[200];

    sprintf(sql, "SELECT * FROM %s WHERE timestamp > %lu;", TABLE_NAME, (unsigned long) ts);

    rc = sqlite3_exec(conn, sql, f, 0, &zErrMsg);

    if (rc != SQLITE_OK) {
        log_event("Error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        log_event("SELECT operation on table %s successfully executed\n", TABLE_NAME);
    }
    return rc;
}
