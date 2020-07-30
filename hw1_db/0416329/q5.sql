SELECT matchType, AVG(matchDuration) FROM matches GROUP BY matchType ORDER BY AVG(matchDuration);
