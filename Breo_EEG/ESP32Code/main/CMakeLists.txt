set(proj_src "app.c"
			 "mod/mod_ble.c"
			 "mod/mod_net.c"
			 "driver/user_http_client.c"
			 "driver/ads1299.c"
			 "driver/filter.c"
			 )

idf_component_register(SRCS "${proj_src}"
                    INCLUDE_DIRS "."
                    			 "./driver"
                    			 "./mod"
                    )

# EMBED_TXTFILES howsmyssl_com_root_cert.pem          
