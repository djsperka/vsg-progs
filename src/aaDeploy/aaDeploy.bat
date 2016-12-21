rem for %%f in ("%1:,=" "%") do copy /f /y %2\%%f %3\%%f
for %%f in (%~1) do copy /y %~2\%%f %~3\%%f
