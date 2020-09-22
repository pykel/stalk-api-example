# stalk-api-example
Example API implementation using Stalk

The goal is to create an example webservice that demonstrates common functions such as:
* http://, https://, ws://, wss://
* routing, with route variables
* middleware chains
** jwt
** x509 peer security verification
** delayed responses - simulate async DB access

## Build
```
(
  export PKG_CONFIG_PATH=$HOME/path-to-stalk-install/lib/pkgconfig ; \
  cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=$PWD/install ../
)

make -j$(nproc)
```

