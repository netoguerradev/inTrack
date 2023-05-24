-- DELETE FROM preceptors WHERE id = 1;

-- UPDATE residents
-- SET preceptor_id = 2
-- WHERE id = 1;

-- INSERT INTO residents (name, password, preceptor_id, residency_id, frequency)
-- VALUES ('joao', '123', 3, 2, 0);

-- UPDATE residents
-- SET name = 'luis'
-- WHERE id = 2;

-- INSERT INTO activities (name, description, max_grade, residency_id) VALUES ('Teste', 'Teste 2', 10, 1);

-- INSERT INTO activities_residents (activity_id, residency_id, user_id, done) VALUES (1, 1, 2, 1);

-- SELECT residents.id, residents.frequency, residents.name, preceptors.name AS preceptor, residencies.residencyName 
-- FROM residents 
-- INNER JOIN preceptors 
-- ON residents.preceptor_id = preceptors.id
-- INNER JOIN residencies
-- ON residents.residency_id = residencies.id;

-- SELECT activities_residents.id, activities_residents.activity_id as activityId, residents.name as residentName, activities.name as activityName, activities.description as activityDescription, activities_residents.done
-- FROM activities_residents
-- INNER JOIN residents
-- ON activities_residents.user_id = residents.id
-- INNER JOIN activities
-- ON activities_residents.activity_id = activities.id
-- WHERE user_id = 1;

-- INSERT INTO activities_residents (activity_id, residency_id, user_id, done) VALUES (1, 1, 1, 1);
-- INSERT INTO activities_residents (activity_id, residency_id, user_id, done) VALUES (1, '1', '1', 1);

-- DELETE from activities_residents where id = 3;