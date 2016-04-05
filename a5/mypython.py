# mypython.py
# Author: Shawn S Hillyer
# Assignmnet 5 - Python Exploration (32 points)
# Completed: April 4, 2016
# Date Due: May 30, 2016

# NOTE: This script uses Python3 syntax for print(), execute using:
# python3 mypython.py

import random
import string
import fcntl


# Create 3 files in the same directory as script each named differently. 
# Files# remain after script executes. Each file contains exactly 10 random 
# characters from the lowercase alphabet with no spaces (e.g. "hoehdgwkdq")
# Terminate each file with a newline character

# Build filenames, open them for writing, write random letter string
fileNames = []
i = 0
while i < 3:
	fileNames.append("random_letters_" + str(i))

	# String of lowercase letters: string.ascii_lowercase, constant defined in 'string' module
	j = 0
	lttrs = []
	while j < 10:
		# Page 256 of PER -- random.choice() returns random item from a sequence
		lttrs.append(random.choice(string.ascii_lowercase)) 
		j += 1
	lttrs.append("\n")
	lttrs = ''.join(lttrs)

	# Open the file and write lttrs to it
	file = open(fileNames[i], "w")
	file.writelines(lttrs)
	file.close();
	i += 1

# Print out the contents of the 3 files
for fileName in fileNames:
	print("Contents of", fileName, ":")
	# Open fileName for reading and print each line in the file
	file = open(fileName, "r")
	for line in file:
		print(line)

# Print out two random integers in the range [1..42] inclusive
# Print out the product of the two numbers

# Citation: Python Essential Reference Page 254
# Seed the random number generator
random.seed() # Seeds with system time if no argument
MIN_RAND = 1
MAX_RAND = 42

# get 2 random int in range [MIN_RAND..MAX_RAND], calc product, print
randA = random.randint(MIN_RAND, MAX_RAND)
randB = random.randint(MIN_RAND, MAX_RAND)
product = randA * randB
print("Two random integers in the range of",MIN_RAND,"-",MAX_RAND,":", "\n",randA, "\n",randB)
print(randA, " *", randB, " =", product)