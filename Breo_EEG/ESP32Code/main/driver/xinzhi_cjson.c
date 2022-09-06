#include "include.h"

int user_parse_json(char *json_data)
{
	cJSON *key_object = NULL;
	cJSON *Item = NULL;
	xinzhi_results_config_t xinzhi_weather_struct;

    printf("\nVersion: %s\n", cJSON_Version());

    key_object = cJSON_Parse(json_data);
    if (!key_object) 
    {
		printf("Error before: [%s]\n",cJSON_GetErrorPtr());
		return  -1;
	}
	printf("%s\n\n", cJSON_Print(key_object));

	cJSON *key_results = cJSON_GetObjectItem(key_object, "results");

	cJSON *item_results = cJSON_GetArrayItem(key_results, 0);
	char *sresults = cJSON_PrintUnformatted(item_results);
	cJSON *results_root = cJSON_Parse(sresults);
	
	cJSON *key_location = cJSON_GetObjectItem(results_root, "location");
	cJSON *key_now = cJSON_GetObjectItem(results_root, "now");
	cJSON *key_last_update = cJSON_GetObjectItem(results_root, "last_update");

{	
	Item = cJSON_GetObjectItem(key_location, "id");
	xinzhi_weather_struct.location_struct.id = cJSON_Print(Item);
	printf("id:%s\n", cJSON_Print(Item));
}

{
	Item = cJSON_GetObjectItem(key_location, "name");
	xinzhi_weather_struct.location_struct.name = cJSON_Print(Item);
	printf("name:%s\n", cJSON_Print(Item));
}

{
	Item = cJSON_GetObjectItem(key_location, "country");
	xinzhi_weather_struct.location_struct.country = cJSON_Print(Item);
	printf("country:%s\n", cJSON_Print(Item));
}

{
	Item = cJSON_GetObjectItem(key_location, "path");
	xinzhi_weather_struct.location_struct.path = cJSON_Print(Item);
	printf("path:%s\n", cJSON_Print(Item));
}

{
	Item = cJSON_GetObjectItem(key_location, "timezone");
	xinzhi_weather_struct.location_struct.timezone = cJSON_Print(Item);
	printf("timezone:%s\n", cJSON_Print(Item));
}

{
	Item = cJSON_GetObjectItem(key_location, "timezone_offset");
	xinzhi_weather_struct.location_struct.timezone_offset = cJSON_Print(Item);
	printf("timezone_offset:%s\n", cJSON_Print(Item));
}

{
	Item = cJSON_GetObjectItem(key_now, "text");
	xinzhi_weather_struct.now_struct.text = cJSON_Print(Item);
	printf("text:%s\n", cJSON_Print(Item));
}

{
	Item = cJSON_GetObjectItem(key_now, "code");
	xinzhi_weather_struct.now_struct.code = cJSON_Print(Item);
	printf("code:%s\n", cJSON_Print(Item));
}

{
	Item = cJSON_GetObjectItem(key_now, "temperature");
	xinzhi_weather_struct.now_struct.temperature = cJSON_Print(Item);
	printf("temperature:%s\n", cJSON_Print(Item));
}

{
	xinzhi_weather_struct.last_update = cJSON_Print(key_last_update);
	printf("last_update:%s\n", cJSON_Print(key_last_update));
}

	cJSON_Delete(key_object);

	return  0;

}




