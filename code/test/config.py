import os
import yaml

# 项目根目录
base_dir = os.path.dirname(os.path.abspath(__file__))

# 测试yaml文件地址
caseyaml_path = base_dir

# 自动生成测试文件地址
generate_case_path = os.path.join(base_dir, 'generate_case')

# 测试文件地址
testcase_path = os.path.join(base_dir, 'testcase')

db_bin_path = os.path.join(os.path.abspath(os.path.join(base_dir, "..")), 'bin')

# 测试日志文件
log_path = os.path.join(base_dir, 'logs')
# error_log_path = os.path.join(base_dir,'error_logs')

# 测试报告地址
report_path = os.path.join(base_dir, 'outputs')

# 测试数据
test_data = {}

def read_test_data():
    data_path = os.path.join(base_dir, "data.yaml")
    print("data_path is " + data_path)
    with open(data_path, encoding='UTF-8')as f:
        d = yaml.load(f, Loader=yaml.SafeLoader)
        return d



if __name__ == '__main__':
    print("db_bin_path is " + db_bin_path )
    print("base_dir is " + base_dir )
    assert len(db_bin_path) != 0
    assert len(base_dir) != 0
    data = read_test_data()
    print("test data is:")
    print(data)
    #assert len(base_dir) != 0