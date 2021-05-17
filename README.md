# aurinstall
A simple AUR helper written in Python.

## **Installation**:
download the script, and run it once with "aurinstall update". It should copy itself to /usr/bin automatically.


## **Searching**:
aurinstall will search the standard repos as well as the AUR. You can specify more than one searchterm to narrow your search.
For example:
`
aurinstall search chromium ungoogled
` will return:
```
aur/ungoogled-chromium-git 88.0.4324.104.1.r3.ga9140d5-1
    A lightweight approach to removing Google web service dependency (master branch)
aur/ungoogled-chromium 88.0.4324.182-1
    A lightweight approach to removing Google web service dependency
```

and adding `git` would only show `ungoogled-chromium-git`.


## **Installing Packages**:
You can install multiple packages with a single command. For example:
`aurinstall install package1 package2 package3 ...`
