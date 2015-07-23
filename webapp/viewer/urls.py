# Copyright (C) 2015 ECMWF

from django.conf.urls import patterns, include, url


urlpatterns = patterns("viewer.views",
    url(r"^list_view$", "list_view", name="list_view"),
    url(r"^set_folder_path$", "set_folder_path", name="set_folder_path"),
    url(r"^$", "home", name="home"),
    #url(r"^test$", "test", name="test"),
    url(r"^search$", "search", name="search"),
    url(r"^get_load_progress$", "get_load_progress", name="get_load_progress"),
    url(r"^serve_png$", "serve_png", name="serve_png"),
    url(r"^serve_detail$", "serve_detail", name="serve_detail"),
    url(r"^get_test_dates$", "get_test_dates", name="get_test_dates"),
    url(r"^serve_hardware$", "serve_hardware", name="serve_hardware"),
    url(r"^serve_source_grids$", "serve_source_grids", name="serve_source_grids"),
    url(r"^serve_target_grids$", "serve_target_grids", name="serve_target_grids"),
    url(r"^serve_grib_dump$", "serve_grib_dump", name="serve_grib_dump"),
    url(r"^serve_grib_compare$", "serve_grib_compare", name="serve_grib_compare"),
    url(r"^serve_grib_ls$", "serve_grib_ls", name="serve_grib_ls"),
    #url(r"^serve_compare$", "serve_compare", name="serve_compare"),
    url(r"^serve_data$", "serve_data", name="serve_data"),
    url(r"^spawn_metview$", "spawn_metview", name="spawn_metview"),
    url(r"^serve_data_frame$", "serve_data_frame", name="serve_data_frame"),
    url(r"^serve_script$", "serve_script", name="serve_script"),
    url(r"^serve_raw_input$", "serve_raw_input", name="serve_raw_input"),
    url(r"^serve_log$", "serve_log", name="serve_log"),
    url(r"^serve_summary$", "serve_summary", name="serve_summary"),
    url(r"^serve_images$", "serve_images", name="serve_images"),
    url(r"^serve_image$", "serve_image", name="serve_image"),
    url(r"^serve_grib_download$", "serve_grib_download", name="serve_grib_download"),
    url(r"^serve_jira_info$", "serve_jira_info", name="serve_jira_info"),
    url(r"^serve_jira_issue$", "serve_jira_issue", name="serve_jira_issue"),
    url(r"^new_jira_issue$", "new_jira_issue", name="new_jira_issue"),
    url(r"^add_jira_comment$", "add_jira_comment", name="add_jira_comment"),


)
