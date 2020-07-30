SELECT matchType, COUNT(matchType) FROM matches WHERE matchType LIKE '%fpp' GROUP BY matchType ORDER BY COUNT(matchType) ASC;
