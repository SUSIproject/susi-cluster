package main

import (
	"encoding/json"
	"errors"
	"flag"
	"log"
	"os"
	"strings"

	"github.com/webvariants/susigo"
)

var conf = flag.String("conf", "/etc/susi/susi-cluster.json", "cluster config file")

var (
	config *Config
	local  *susigo.Susi
	nodes  = make(map[string]*susigo.Susi)
)

type Config struct {
	Local *NodeConfig
	Nodes []*NodeConfig
}

func (cfg *Config) GetNode(id string) *NodeConfig {
	for _, node := range cfg.Nodes {
		if node.ID == id {
			return node
		}
	}
	return nil
}

type NodeConfig struct {
	ID                 string         `json:"id"`
	Addr               string         `json:"addr"`
	Cert               string         `json:"cert"`
	Key                string         `json:"key"`
	ForwardConsumers   []*EventConfig `json:"forwardConsumers"`
	ForwardProcessors  []*EventConfig `json:"forwardProcessors"`
	RegisterConsumers  []*EventConfig `json:"registerConsumers"`
	RegisterProcessors []*EventConfig `json:"registerProcessors"`
}

type EventConfig struct {
	Topic       string `json:"topic"`
	StripSuffix string `json:"stripSuffix"`
}

func loadConfig() error {
	f, err := os.Open(*conf)
	if err != nil {
		return err
	}
	defer f.Close()
	decoder := json.NewDecoder(f)
	err = decoder.Decode(&config)
	if err != nil {
		return err
	}
	return nil
}

func setupLocal() error {
	susi, err := susigo.NewSusi(config.Local.Addr, config.Local.Cert, config.Local.Key)
	if err != nil {
		return err
	}
	local = susi
	local.Wait()
	log.Print("successfully connected to local susi-core")
	return nil
}

func setupNodes() error {
	var occuredError error
	for _, node := range config.Nodes {
		log.Printf("setup connection to node %v", node.ID)
		susi, err := susigo.NewSusi(node.Addr, node.Cert, node.Key)
		if err != nil {
			occuredError = errors.New("error creating susi client for " + node.ID + ": " + err.Error())
			continue
		}
		nodes[node.ID] = susi
		setupForwardConsumers(node)
		setupForwardProcessors(node)
		setupRegisterConsumers(node)
		setupRegisterProcessors(node)
	}
	return occuredError
}

func setupForwardConsumers(node *NodeConfig) {
	nodeSusi := nodes[node.ID]
	for idx := range node.ForwardConsumers {
		eventConfig := node.ForwardConsumers[idx]
		local.RegisterConsumer(eventConfig.Topic, func(event *susigo.Event) {
			if eventConfig.StripSuffix != "" {
				event.Topic = strings.TrimSuffix(event.Topic, eventConfig.StripSuffix)
			}
			log.Printf("forward consumer event %v as %v to %v", eventConfig.Topic, event.Topic, node.ID)
			event.AddHeader("Event-Control", "No-Ack")
			nodeSusi.Publish(*event, nil)
		})
	}
}

func setupForwardProcessors(node *NodeConfig) {
	nodeSusi := nodes[node.ID]
	for idx := range node.ForwardProcessors {
		eventConfig := node.ForwardProcessors[idx]
		local.RegisterProcessor(eventConfig.Topic, func(event *susigo.Event) {
			if eventConfig.StripSuffix != "" {
				event.Topic = strings.TrimSuffix(event.Topic, eventConfig.StripSuffix)
			}
			log.Printf("forward processor event %v as %v to %v", eventConfig.Topic, event.Topic, node.ID)
			nodeSusi.Publish(*event, func(remoteEvent *susigo.Event) {
				remoteEvent.Topic = eventConfig.Topic
				local.Ack(remoteEvent)
			})
		})
	}
}

func setupRegisterConsumers(node *NodeConfig) {
	nodeSusi := nodes[node.ID]
	for idx := range node.RegisterConsumers {
		eventConfig := node.RegisterConsumers[idx]
		nodeSusi.RegisterConsumer(eventConfig.Topic, func(remoteEvent *susigo.Event) {
			if eventConfig.StripSuffix != "" {
				remoteEvent.Topic = strings.TrimSuffix(remoteEvent.Topic, eventConfig.StripSuffix)
			}
			log.Printf("got consumer event %v from %v, publish as %v", eventConfig.Topic, node.ID, remoteEvent.Topic)
			local.Publish(*remoteEvent, nil)
		})
	}
}

func setupRegisterProcessors(node *NodeConfig) {
	nodeSusi := nodes[node.ID]
	for idx := range node.RegisterProcessors {
		eventConfig := node.RegisterProcessors[idx]
		nodeSusi.RegisterProcessor(eventConfig.Topic, func(remoteEvent *susigo.Event) {
			if eventConfig.StripSuffix != "" {
				remoteEvent.Topic = strings.TrimSuffix(remoteEvent.Topic, eventConfig.StripSuffix)
			}
			log.Printf("got processor event %v from %v, publish as %v", eventConfig.Topic, node.ID, remoteEvent.Topic)
			local.Publish(*remoteEvent, func(localEvent *susigo.Event) {
				localEvent.Topic = eventConfig.Topic
				nodeSusi.Ack(localEvent)
			})
		})
	}
}

func main() {
	flag.Parse()
	err := loadConfig()
	if err != nil {
		log.Fatal(err)
	}
	err = setupLocal()
	if err != nil {
		log.Fatal(err)
	}
	err = setupNodes()
	if err != nil {
		log.Fatal(err)
	}
	select {}
}
