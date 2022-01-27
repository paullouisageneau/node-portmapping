const nodePortMapping = require('../lib/index');

function delay(ms) {
    return new Promise((resolve) => { setTimeout(resolve, ms); });
}

async function main() {
    const config = {
        logLevel : 'Debug',
    };

    nodePortMapping.init(config);

    const spec = {
        protocol : 'TCP',
        internalPort : 8080,
    };

    const mapping = nodePortMapping.createMapping(spec, (info) => {
        if (info.state == 'Success') {
            console.log(`${info.externalHost}:${info.externalPort}`);
        }
    });

    await delay(10000)

    mapping.destroy();

    await nodePortMapping.cleanup();
}

main().then(() => process.exit(0));
