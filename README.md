# aurinstall
A simple AUR helper written in C.
It can be configured using the source code.

## **Installation**:
You'll need to install the packages `curl`, `json-c`, and `gcc`.

Download the source code and run `sudo make install`.

## **Searching**:
aurinstall only searches the AUR. You can specify more than one searchterm to narrow your search.
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

## **Cleaning the Cache**
You can clean the cache by running `aurinstall clean`.
By default the cache is located at `~/.cache/aurinstall/`

## **Removing Packages**
You can remove packages the same way you'd install them, just replace `install` with `remove`.
