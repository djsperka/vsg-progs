
CString TrimLine(char input[100])
{
	CString output;
	int pos;

	output=input;
	pos=output.Find("\n");
	output=output.Left(pos);
	output.TrimRight();

	return output;
}
