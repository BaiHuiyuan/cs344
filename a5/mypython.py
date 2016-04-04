# mypython.py
# Author: Shawn S Hillyer
# Assignmnet 5 - Python Exploration (32 points)
# Date Due: May 30, 2016
import random
import fcntl


# Create 3 files in the same directory as your script
# each named differently (filename up to author). Files
# remain after script executes.
# Each file contains exactly 10 random characters from
# the lowercase alphabet with no spaces (e.g. "hoehdgwkdq")
# Terminate each file with a newline character

# Citation for files with OS: Python Essential Reference 347+

# Build filenames, open them for writing, write random letter string
fileNames = []
i = 0
while i < 3:
	fileNames.append("randLttrs" + str(i))
	print(fileNames[i])

	i = i + 1



# Print out the contents of the 3 files
#for line in 


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
print(randA)
print(randB)
print(product)