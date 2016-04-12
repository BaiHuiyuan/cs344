fd = open("myout.data", O_WRONLY | O_CREATE|O_TRUNC, 0644);

if (fd == -1 ) {
	perror("open");
	exit(1);
}

fd2 = dup2(fd, 1); // change where stdout is pointing -- make it point to where our newly opened output file points
if (fd2 == -1) {
	perror("dup2");
	exit(2);
}

// exec here...

// And for redirecting stdin:

fd = open("myin.data", O_RDONLY);
if (fd == -1 )
{
	perror("open");
	exit(1);
}

fd2 = dup2(fd, 0);
if (fd2 == -1) {
	perror("dup2");
	exit(2);
}

// exec....