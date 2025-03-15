from behave import *

import subprocess
import os.path
import os

@given('A folder with YAML files')
def step_impl(context):
    print(f'current path is { os.getcwd() }')
    pass    

@when('tsn_gen folder argument is ./tests/dummy-proto-folder')
def step_impl(context):
    global error
    error = subprocess.run(['../../build/tsn_gen', '-P../../test-dummy-proto-folder'])
    pass

@when('tsn_gen should return an error code 254')
def step_impl(context):
    assert error == 254
