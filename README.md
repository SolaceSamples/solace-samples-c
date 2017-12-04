# Getting Started Examples
## Solace C API

The "Getting Started" tutorials will get you up to speed and sending messages with Solace technology as quickly as possible. There are three ways you can get started:

- Follow [these instructions]({{ site.links-solaceCloud-setup }}){:target="_top"} to quickly spin up a cloud-based Solace messaging service for your applications.
- Follow [these instructions]({{ site.docs-vmr-setup }}){:target="_top"} to start the Solace VMR in leading Clouds, Container Platforms or Hypervisors. The tutorials outline where to download and how to install the Solace VMR.
- If your company has Solace message routers deployed, contact your middleware team to obtain the host name or IP address of a Solace message router to test against, a username and password to access it, and a VPN in which you can produce and consume messages.

## Contents

This repository contains code and matching tutorial walk throughs for five different basic Solace messaging patterns. For a nice introduction to the Solace API and associated tutorials, check out the [tutorials home page](https://solacesamples.github.io/solace-samples-c/).

## Checking out and Building

To check out the project and build it, do the following:

  1. clone this GitHub repository
  1. `cd solace-samples-c`
 
### Download the Solace C API

The C API library can be [downloaded here](http://dev.solace.com/downloads/). The build instructions below assume you have unpacked the tar file into `src` subdirectory of your GitHub repository. 

### Build the Samples

Building these examples is simple. The following provides an example using Linux. For ideas on how to build on other platforms you can consult the README of the C API library.

```
gcc -g -Wall -I ../include -L ../lib -lsolclient HelloWorldPub.c -o HelloWorldPub
gcc -g -Wall -I ../include -L ../lib -lsolclient os.c HelloWorldSub.c -o HelloWorldSub
```

## Running the Samples

To try individual samples, build the project from source and then run samples like the following:

On **Linux**:

```
$ LD_LIBRARY_PATH=../lib:$LD_LIBRARY_PATH ./HelloWorldSub <<HOST_ADDRESS>>

```

See the [tutorials](https://solacesamples.github.io/solace-samples-c/) for more details.

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct, and the process for submitting pull requests to us.

## Authors

See the list of [contributors](https://github.com/SolaceSamples/solace-samples-c/contributors) who participated in this project.

## License

This project is licensed under the Apache License, Version 2.0. - See the [LICENSE](LICENSE) file for details.

## Resources

For more information try these resources:

- The Solace Developer Portal website at: http://dev.solace.com
- Get a better understanding of [Solace technology](http://dev.solace.com/tech/).
- Check out the [Solace blog](http://dev.solace.com/blog/) for other interesting discussions around Solace technology
- Ask the [Solace community.](http://dev.solace.com/community/)
