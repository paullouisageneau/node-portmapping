# Multi-protocol NAT Port Mapping for Node.js

node-portmapping allows to forward ports on Network Address Translators (NAT). It implements the protocols PCP, NAT-PMP, and UPnP, and automatically detects which one to use.

This project consists in Node.js bindings for [libplum](https://github.com/paullouisageneau/libplum). Please check [libplum](https://github.com/paullouisageneau/libplum) for more details.

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

