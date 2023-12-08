#ifndef API_COMMUNICATION_H
#define API_COMMUNICATION_H

/* Constants that aren't configurable in menuconfig */
#define WEB_SERVER "192.168.1.173"
#define WEB_PORT "5000"
#define SENSOR_PREFIX_PATH "/api/updatedb"
#define FORECASTING_PREFIX_PATH "api/getcommand"
 
void encode_sensor_data(char*, float , float , float );
void http_send_data(float, float, float);
void http_request_command(int *, int*);

#endif //API_COMMUNICATION_H