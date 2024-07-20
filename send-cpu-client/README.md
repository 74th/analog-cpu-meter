# CPU リソースを送るプログラム

```
go build .
./send-cpu-usage -h http://192.168.1.105 -v -i 500ms
```

## Systemd で動かす

[systemd/send-cpu-usage.service](systemd/send-cpu-usage.service) の `ExecStart` の`-h http://192.168.1.105` のホスト名を編集してから以下のコマンドを実行する。

```
go build .
sudo cp ./send-cpu-usage /usr/local/bin/
sudo cp systemd/send-cpu-usage.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable send-cpu-usage.service
sudo systemctl start send-cpu-usage.service
sudo systemctl status send-cpu-usage.service
```

送られていないようであれば、`ExecStart`に記述する引数に`-s`を削除、`-v`を追加してログを確認してください。
