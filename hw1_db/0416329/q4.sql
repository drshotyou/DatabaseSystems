SELECT Id, AVG(kills) FROM `player_statistic`p INNER JOIN (SELECT matchId FROM `matches` WHERE numGroups<=10)m GROUP BY ABG(kills) DESC LIMIT 20; 
