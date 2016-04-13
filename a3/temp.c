					if (!redir_input && bg_mode) {
						input_file = devnull;
					}

					if (redir_output && bg_mode) {
						output_file = devnull;
					}

					fd_in = open(input_file, O_RDONLY);
					if (fd_in == -1) {
						perror("open");
						exit(1);
					}
					fd_in2 = dup2(fd_in, 0); // 0 = stdin
					if (fd_in2 == -1) {
						perror("dup2");
						exit(2);
					}

					fd_out = open(output_file, O_WRONLY | O_TRUNC | O_CREAT, 0644);
					if (fd_out == -1) {
						perror("open");
						exit(1);
					}

					fd_out2 = dup2(fd_out, 1); // 1 = stdout
					if (fd_out2 == -1) {
						perror("dup2");
						exit(2);
					}					