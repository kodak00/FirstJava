{

	FS=" "
    #string = "2017-04-01 21:43:40.232473+09:00"
    #string = "2017-04-01 21:58:59.123456+09:00"
	string = $2$3

    groups = "^(....)-(..)-(..)(..):(..):(..).*"

    format = "\\1 \\2 \\3 \\4 \\5 \\6"
    datespec = gensub(groups, format, "", string)
    timestampA = mktime(datespec)
	#printf("%d\n", timestampA);
	#printf("%d\n", timestampA*1000000 + substr(string,21,6));

    format = "\\1 \\2 \\3 \\4 00 00"
    datespec = gensub(groups, format, "", string)
    timestampB = mktime(datespec)
	#printf("%d\n", timestampB);
	#printf("%d\n", timestampB*1000000 + 0);

	for(i=1; i<=12; i++)
	{
		if( timestampA*1000000 + substr(string,21,6) >= (timestampB-5*i*60)*1000000 + 0 &&\
			timestampA*1000000 + substr(string,21,6) < (timestampB+5*i*60)*1000000 + 0)
		{
			#print string
			#print strftime("%F-%H.%M.%S", timestampB+5*i*60)

			filename = sprintf("KMA_LGT_NX1_%s.asc", strftime("%Y%m%d%H%M%S", timestampB+5*i*60))
			print $0 >> filename
			break
		}
	}

}

