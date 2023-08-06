#import subprocess
import allure
import config


@allure.feature('插入测试')
class TestInsert:
    def test_insert_one(self):
        #f = subprocess.Popen("python test.py", shell=True, stdin=subprocess.PIPE)
        assert config.data
        