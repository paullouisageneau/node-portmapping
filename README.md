# Multi-protocol NAT Port Mapping for Node.js

[![License: LGPL v2.1 or later](https://img.shields.io/badge/License-LGPL_v2.1_or_later-blue.svg)](https://www.gnu.org/licenses/old-licenses/lgpl-2.1.en.html)
[![NPM package](https://img.shields.io/npm/v/node-portmapping?color=green)](https://www.npmjs.com/package/node-portmapping)

node-portmapping allows to forward ports on Network Address Translators (NAT). It implements the protocols PCP, NAT-PMP, and UPnP, and automatically detects which one to use.

This project consists in Node.js bindings for [libplum](https://github.com/paullouisageneau/libplum), which is licensed under LGPLv2.1 or later.

## Example
```js
const nodePortMapping = require('node-portmapping');

nodePortMapping.init();

const mapping = nodePortMapping.createMapping(8080, (info) => {
    if (info.state == 'Success') {
        console.log(`${info.externalHost}:${info.externalPort}`);
    }
});
```

## Install
```sh
$ npm install node-portmapping
```

