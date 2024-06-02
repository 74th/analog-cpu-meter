package main

import (
	"bytes"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"time"

	"github.com/shirou/gopsutil/v4/cpu"
	"github.com/shirou/gopsutil/v4/mem"
)

type Usage struct {
	cpu    float64
	memory float64
}

type SendData struct {
	Value int `json:"value"`
}

var isVerbose bool
var serverHost string
var interval time.Duration

func getUsage() (Usage, error) {
	memoryUsage, err := mem.VirtualMemory()
	if err != nil {
		return Usage{}, err
	}

	if isVerbose {
		log.Printf("Total: %v, Free:%v, UsedPercent:%f%%\n", memoryUsage.Total, memoryUsage.Free, memoryUsage.UsedPercent)
	}

	totalCpuUsage, err := cpu.Percent(time.Millisecond*100, false)
	if err != nil {
		return Usage{}, err
	}

	if len(totalCpuUsage) != 1 {
		panic("cpu usage is empty")
	}

	if isVerbose {
		log.Printf("CPU: %2f%%\n", totalCpuUsage[0])
	}

	return Usage{
		cpu:    totalCpuUsage[0],
		memory: memoryUsage.UsedPercent,
	}, nil
}

func postUsage(u Usage) error {
	data := SendData{Value: int(u.cpu)}

	buf := bytes.NewBuffer(nil)
	err := json.NewEncoder(buf).Encode(&data)
	if err != nil {
		return err
	}

	if isVerbose {
		log.Printf("request %s", serverHost)
	}

	req, err := http.NewRequest("POST", serverHost, buf)
	if err != nil {
		return err
	}

	res, err := http.DefaultClient.Do(req)
	if err != nil {
		return err
	}

	if res.StatusCode != 200 {
		resBody, _ := io.ReadAll(res.Body)
		return fmt.Errorf("status code is not 200: %d, body: %s", res.StatusCode, resBody)
	}

	if isVerbose {
		resBody, _ := io.ReadAll(res.Body)
		log.Printf("status code is 200: %d, body: %s", res.StatusCode, resBody)
	}

	return nil
}

func main() {
	flag.BoolVar(&isVerbose, "v", false, "verbose")
	flag.StringVar(&serverHost, "h", "", "server host")
	flag.DurationVar(&interval, "i", time.Second, "interval")
	flag.Parse()

	if serverHost == "" {
		fmt.Printf("usage -h <server host>\n")
		os.Exit(1)
	}

	for {
		time.Sleep(interval)

		usage, err := getUsage()
		if err != nil {
			log.Printf("getUsageError: %s", err)
			continue
		}

		err = postUsage(usage)
		if err != nil {
			log.Printf("postUsageError: %s", err)
			continue
		}
	}
}
