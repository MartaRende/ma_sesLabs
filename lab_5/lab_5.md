# Lab 5 - SETUID bit

## Question 1
 To enable ext4 filesystem support on the image, you can go into the linux kernel configuration (`make linux-menuconfig`), and then go to `Filesystems -> ext4` and enable all options for ext4 filesystem support.

The setuid bit, if set, will change the user executing the script to the one that owns it. In our example, as we created the script with `root`and the setuid bit is set, when we run the script, the created process is ran by `root`, which allow it to read the content of the password file.

## Question 2

To protect against those attacks without limiting the capabilities of the system, we can : enforce that for every mounted filesystem except `/`, we mount it using the `nosuid` option, which ignore the setuid bit on all files in this partition.
