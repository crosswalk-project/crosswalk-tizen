// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <iostream>

#include "bundle/extension_client.h"

using namespace std;
using namespace wrt;

int main(int argc, char* argv[]) {

  if (argc < 2) {
    fprintf(stderr, "uuid is requried.\n");
  }

  ExtensionClient extension_client;

  extension_client.Initialize(argv[1]);

  string instance_id = extension_client.CreateInstance("tizen", NULL);
  extension_client.DestroyInstance(instance_id);
  cout << instance_id << endl;

  return 0;
}
