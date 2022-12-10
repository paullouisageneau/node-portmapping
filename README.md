# Multi-protocol NAT Port Mapping for Node.js

[![License: MPL 2.0](https://img.shields.io/badge/License-MPL_2.0-blue.svg)](https://www.mozilla.org/en-US/MPL/2.0/)
[![NPM package](https://img.shields.io/npm/v/node-portmapping?color=green)](https://www.npmjs.com/package/node-portmapping)

node-portmapping allows to forward ports on Network Address Translators (NAT). It implements the protocols PCP, NAT-PMP, and UPnP, and automatically detects which one to use.

This project consists in Node.js bindings for [libplum](https://github.com/paullouisageneau/libplum), which is licensed under MPL 2.0.

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

