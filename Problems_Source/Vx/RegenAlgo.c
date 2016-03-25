	
	srand(time(0));
	//if (argc % 2 != 1)
	//{
	//	fputs("Not enough parameters.", stderr);
	//}
	for (int i = 1; i < argc; ++i)
	{
		int iRand = GenRandomCharString();
		int asd[2] = { iRand, 0 };
		printf_s("%s %i %i %s %x %x\n", argv[i], *(int*)argv[i] ^ iRand, iRand, asd, *(int*)argv[i] ^ iRand, iRand);
	}

	system("pause");
	return 0;