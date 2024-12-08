SET MYPATH=turning_holders
python 2_yg1_create_sm.py %MYPATH% && python 3_yg1_load_db_ctd.py %MYPATH% && python 4_yg1_ctd_from_sm_fill.py %MYPATH%