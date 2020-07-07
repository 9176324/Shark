git add *
$commit = Get-Date -Format 'yyyy-MM-dd-HH-mm-ss'
git commit -m $commit
git push -u GitHub master
pause