import sys
import time
import json
from urllib import request
from urllib import error

API_HOST = "http://192.168.1.105"


def putValue(path: str, value: int):
    api = f"/{path}"
    request_url = API_HOST + api

    data = {"value": value}

    header = {"Content-Type": "application/json"}

    try:
        req_body = json.dumps(data)
        print("POST", request_url)
        print("REQ:", req_body)
        request_post = request.Request(
            url=request_url, headers=header, method="POST", data=req_body.encode()
        )
        with request.urlopen(request_post) as res:
            res_body = res.read().decode()
        print("RES:", res_body)
    except error.HTTPError as e:
        print(f"{e.code} RES:", e.read().decode())
    except error.URLError as e:
        print("通信失敗")

def putValues(cpu: int, mem: int):
    request_url = API_HOST + "/values"

    data = {"cpu": cpu, "mem": mem}

    header = {"Content-Type": "application/json"}

    try:
        req_body = json.dumps(data)
        print("POST", request_url)
        print("REQ:", req_body)
        request_post = request.Request(
            url=request_url, headers=header, method="POST", data=req_body.encode()
        )
        with request.urlopen(request_post) as res:
            res_body = res.read().decode()
        print("RES:", res_body)
    except error.HTTPError as e:
        print(f"{e.code} RES:", e.read().decode())
    except error.URLError as e:
        print("通信失敗")

def main1():
    while True:
        values = [0, 10, 50, 75, 100]
        for value in values:
            print(f"put CPU: {value}")
            putValue("cpu", value)
            time.sleep(0.5)
            print(f"put MEM: {value}")
            putValue("mem", value)
            time.sleep(0.5)

def main2():
    while True:
        values = [0, 10, 50, 75, 100]
        for n,_ in enumerate(values):
            print(f"put CPU: {values[n]} MEM: {values[n-1]}")
            putValues(values[n], values[n-1])
            time.sleep(0.5)


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "1":
        main1()
    else:
        main2()
