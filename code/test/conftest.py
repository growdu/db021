import pytest
import config


@pytest.fixture(scope='module', autouse=True)
def init_data():
    """
    初始化，获取初测试表数据
    """
    config.data = config.read_test_data()
    print("test_data is:")
    for d in config.data:
        print("id:" + str(d['id']) + ",name: " + d['name'] + "email:" + d['email'])
        