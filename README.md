# Getting Started Examples
## Solace C API

The "Getting Started" tutorials will get you up to speed and sending messages with Solace technology as quickly as possible. There are three ways you can get started:

- Follow [these instructions](https://cloud.solace.com/learn/group_getting_started/ggs_signup.html) to quickly spin up a cloud-based Solace messaging service for your applications.
- Follow [these instructions](https://docs.solace.com/Solace-SW-Broker-Set-Up/Setting-Up-SW-Brokers.htm) to start the Solace VMR in leading Clouds, Container Platforms or Hypervisors. The tutorials outline where to download and how to install the Solace VMR.
- If your company has Solace message routers deployed, contact your middleware team to obtain the host name or IP address of a Solace message router to test against, a username and password to access it, and a VPN in which you can produce and consume messages.

## Contents

This repository contains code and matching tutorial walk throughs for the basic Solace messaging patterns. For a nice introduction to the Solace API and associated tutorials, check out the [tutorials home page](https://dev.solace.com/samples/solace-samples-c/).

## Prerequisites ##

### On Alpine Linux
- make
- gcc compiler
- musl
- musl-dev
- openssl
- openssl-dev
- libressl-dev

### On Linux (non-alpine linux)/Mac
- gcc compiler

### On Windows
- Visual Studio 2015 and above. (Tested on VS2015/2019/2022)  

## Checking out and Building

To check out the project and build it, do the following:

  1. `git clone` this GitHub repository
  2. `cd solace-samples-c`
 
### Build the Samples

1. `cd build`  

2. On Alpine Linux:
```
    build$ ./build_intro_linux_musl_x64.sh
```
3. On Linux:  
```
    build$ ./build_intro_linux_xxx.sh
```
Note: it s important to set the environment on Alpine Linux and Linux, see in next step [Running the Samples](#running-the-samples).

4. On Mac:
```
    build$ ./build_intro_mac_xxx.sh
```
5. On Windows, you can either build the source code from Visual Studio IDE or from DOS command prompt.   
To build from the IDE, you will need to go to `build\intro\win\VS2015` and double-click on `intro.sln`.  
To build from DOS prompt, you must launch the appropriate Visual Studio Command Prompt and then run the `build_intro_win_xxx.bat`

### Running the Samples

To try individual samples, build the project from source and then run samples like the following:

1. `cd ../bin`

2. On Linux and Alpine-linux:
```
    bin$ source ./setenv.sh
    bin$ ./TopicPublisher
```

3. On Mac and Windows, you can run the sample app straight-away.  

See the [tutorials](https://dev.solace.com/samples/solace-samples-c/) for more details.

## Contributing

Please read [CONTRIBUTING](CONTRIBUTING.md) for details on our code of conduct, and the process for submitting pull requests to us.

## Authors

See the list of [contributors](https://github.com/SolaceSamples/solace-samples-c/contributors) who participated in this project.

## License

This project is licensed under the Apache License, Version 2.0. - See the [LICENSE](LICENSE) file for details.

## Resources

For more information try these resources:

- The Solace Developer Portal website at: http://dev.solace.com
- Get a better understanding of [Solace technology](https://solace.com/products/tech/).
- Check out the [Solace blog](http://dev.solace.com/blog/) for other interesting discussions around Solace technology
- Ask the [Solace community.](https://solace.community/)
