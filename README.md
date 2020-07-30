# Symblock

A unique windows kernel driver to protect your system from privilege escalation due to symbolic link exploits.

### Usage
Tested on Windows 10 and Windows 7.

  - Compile the driver
  - Update .inf file according to your usage
  - Install .inf file by right click-> install
  - Load the driver using `net start drivername`
You can use debugview tool to get the debug message generated

### Technical details
The driver blocks attempt to create directory junction/mount point to Object directory `RPC Control` and `BaseNamedObjects` which is responsible for the symlink attack. <br />
More details can be found here: [nixhacker.com](https://nixhacker.com/mitigate-and-detect-local-privilege-escalation-through-symbolic-links/)

### Coverage
There are more then hundreds of CVE's per year that are present due to symbolic link attacks. Here are few of them:
| CVE        | Product           | CVE  | Product |
| ------------- |:-------------:| :-----:|-------:|
|  CVE-2019-1161     | Windows defender | CVE-2019-1142 | .Net Framework |
| CVE-2020-1088    | Windows error Reporting     |   CVE-2020-1021 | Windows error Reporting |
| CVE-2019-3726 | Dell DUP Fraamework    |    CVE-2020-7250 | Mcafee ENS |
| CVE-2020-7257 | Mcafee ENS   |    CVE-2020-0683 | Windows installer |
| CVE-2019-19793 | AppGate SDP    |    CVE-2020-13162 | Pulse Secure Client |
| CVE-2017-13680 | Symantec Endpoint    |    CVE-2019-11396 | Avira Antivirus |
| CVE-2020-0863 | Windows Diagnosis service    |    CVE-2020-0776 | Windows AppX Server |
| CVE-2018-12177 | Intel PROset Wireless    |    CVE-2019-1316 | Microsoft Windows Setup |
| CVE-2019-1405 | WIndows UPnP    |    CVE-2020-13149 | MSI Dragon Center |
| CVE-2019-0841 | Windows AppX    |    CVE-2019-0636| Windows installer |
| Many more | Many more | Many more | Many more



### Screenshot:

<a href="https://ibb.co/BzNQPn6"><img src="https://i.ibb.co/QbdTcDH/symlink-detected.png" alt="symlink-detected" border="0"></a><br />


