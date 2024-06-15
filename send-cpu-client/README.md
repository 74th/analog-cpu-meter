# CPU リソースを送るプログラム

```
go build .
./send-cpu-usage -h http://192.168.1.105 -v -i 500ms
```

## Systemd で動かす

[systemd/send-cpu-usage.service](systemd/send-cpu-usage.service) の `ExecStart` の`-h http://192.168.1.105` のホスト名を編集してから以下のコマンドを実行する。

```
go build .
sudo cp ./send-cpu-usage /usr/loca/bin/
sudo cp systemd/send-cpu-usage.service /etc/systemd/system/
sudo systemd daemon-reload
sudo systemd enable send-cpu-usage.service
sudo systemd start send-cpu-usage.service
sudo systemd status send-cpu-usage.service
```

送られていないようであれば、`ExecStart`に記述する引数に`-v`を追加してログを確認してください。
