/*
 * Copyright (c) 2016, Tino Rusch
 *
 * This file is released under the terms of the MIT license. You can find the
 * complete text in the attached LICENSE file or online at:
 *
 * http://www.opensource.org/licenses/mit-license.php
 *
 * @author: Tino Rusch (tino.rusch@webvariants.de)
 */

#include "susi/ClusterComponent.h"
#include "susi/BaseApp.h"

class ClusterApp : public Susi::BaseApp {
protected:
  std::shared_ptr<Susi::ClusterComponent> _clusterComponent;

public:
  ClusterApp(int argc, char **argv) : Susi::BaseApp{argc, argv} {}
  virtual ~ClusterApp() {}
  virtual void start() override {
    _clusterComponent.reset(
        new Susi::ClusterComponent{*_susi, _config["component"]});
  }
};

int main(int argc, char *argv[]) {
  try {
    ClusterApp app{argc, argv};
    app.start();
    app.join();
  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
    return 1;
  }
  return 0;
}
