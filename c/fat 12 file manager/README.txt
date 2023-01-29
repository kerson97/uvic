==diskinfo, disklist, diskget, diskput Documentation==
Contributors: Carson Seidel, V00933347

This set of programs can display information about a FAT12 image, list the files/directories in the FAT12 image, copy a file from the image to the current directory, and write a file from the current directory to the image.

How to use these programs
Step 1: compile test files that you want to use, put them in current directory
Step 2: execute "make" in terminal to compile diskinfo, disklist,diskget and diskput

A) diskinfo
1) execute "./diskinfo testImage.IMA" in terminal to run diskinfo on testImage.IMA (where testImage.IMA can be any IMA in the same dir)

B) disklist
1) execute "./disklist testImage.IMA" in terminal to run disklist on testImage.IMA (where testImage.IMA can be any IMA in the same dir)

C) diskget
1) execute "./diskget filename.extension" where filename.extension is the name and extension of the file in the IMA that you wish to copy to current directory

D)diskput
1) execute "./diskput testImage.IMA /PATH/ filename.extension" where testImage is an IMA in the current directory and /PATH/ is the path in which filename.exension is to be written

OR

1)i) execute "./diskput testImage.IMA filename.extension" where testImage is an IMA in the current directory, this will write filename.exension to the root directory



Important notes for diskput:
1) ensure the cases match for the file to be written, ie if you want to write "input0.txt", do NOT write INPUT0.TXT. This was a bug I wasn't able to sort out.

Other: Included unmodified testNew.IMA and input0.txt as options for convenient testing

