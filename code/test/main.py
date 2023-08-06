#!/usr/bin/python
import os
import pytest
import config

def run_testcase():
    pytest.main([config.testcase_path,'-sv','--alluredir=outputs/report','--clean-alluredir'])
    os.system('allure generate outputs/report -o outputs/html --clean')
    os.system('allure serve outputs/report')
    
    
if __name__ == '__main__':
    run_testcase()
    