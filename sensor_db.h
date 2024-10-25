/**
 * \author Mustafa Ekici
 */

#ifndef _SENSOR_DB_H_
#define _SENSOR_DB_H_

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "sbuffer.h"
#include "main.h"
#include "errmacros.h"
#include "config.h"

// stringify preprocessor directives using 2-level preprocessor magic
// this avoids using directives like -DDB_NAME=\"some_db_name\"
#define REAL_TO_STRING(s) #s
#define TO_STRING(s) REAL_TO_STRING(s)    //force macro-expansion on s before stringify s

#ifndef DB_NAME
#define DB_NAME Sensor.db
#endif

#ifndef TABLE_NAME
#define TABLE_NAME SensorData
#endif

#define DBCONN sqlite3

typedef int (*callback_t)(void *, int, char **, char **);

/**
 * Make a connection to the database server
 * Create (open) a database with name DB_NAME having 1 table named TABLE_NAME  
 * \param clear_up_flag if the table existed, clear up the existing data when clear_up_flag is set to 1
 * \return the connection for success, NULL if an error occurs
 */
DBCONN *init_connection(char clear_up_flag);

/**
 * Disconnect from the database server
 * \param conn pointer to the current connection
 */
void disconnect(DBCONN *conn);

/**
 * Write an INSERT query to insert a single sensor measurement
 * \param conn pointer to the current connection
 * \param id the sensor id
 * \param value the measurement value
 * \param ts the measurement timestamp
 * \return zero for success, and non-zero if an error occurs
 */
int insert_sensor(DBCONN *conn, sensor_id_t id, sensor_value_t value, sensor_ts_t ts);

/*
 * Reads continiously all data from the shared buffer data structure and stores this into the database
 * When *buffer becomes NULL the method finishes. This method will NOT automatically disconnect from the db
 */
void storagemgr_parse_sensor_data(DBCONN *conn, sbuffer_t **buffer);

/**
  * Write a SELECT query to select all sensor measurements in the table 
  * The callback function is applied to every row in the result
  * \param conn pointer to the current connection
  * \param f function pointer to the callback method that will handle the result set
  * \return zero for success, and non-zero if an error occurs
  */
int find_sensor_all(DBCONN *conn, callback_t f);

/**
 * Write a SELECT query to return all sensor measurements having a temperature of 'value'
 * The callback function is applied to every row in the result
 * \param conn pointer to the current connection
 * \param value the value to be queried
 * \param f function pointer to the callback method that will handle the result set
 * \return zero for success, and non-zero if an error occurs
 */
int find_sensor_by_value(DBCONN *conn, sensor_value_t value, callback_t f);

/**
 * Write a SELECT query to return all sensor measurements of which the temperature exceeds 'value'
 * The callback function is applied to every row in the result
 * \param conn pointer to the current connection
 * \param value the value to be queried
 * \param f function pointer to the callback method that will handle the result set
 * \return zero for success, and non-zero if an error occurs
 */
int find_sensor_exceed_value(DBCONN *conn, sensor_value_t value, callback_t f);

/**
 * Write a SELECT query to return all sensor measurements having a timestamp 'ts'
 * The callback function is applied to every row in the result
 * \param conn pointer to the current connection
 * \param ts the timestamp to be queried
 * \param f function pointer to the callback method that will handle the result set
 * \return zero for success, and non-zero if an error occurs
 */
int find_sensor_by_timestamp(DBCONN *conn, sensor_ts_t ts, callback_t f);

/**
 * Write a SELECT query to return all sensor measurements recorded after timestamp 'ts'
 * The callback function is applied to every row in the result
 * \param conn pointer to the current connection
 * \param ts the timestamp to be queried
 * \param f function pointer to the callback method that will handle the result set
 * \return zero for success, and non-zero if an error occurs
 */
int find_sensor_after_timestamp(DBCONN *conn, sensor_ts_t ts, callback_t f);

#endif /* _SENSOR_DB_H_ */
