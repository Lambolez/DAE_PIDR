create or replace
trigger trigger_algorithm
before insert on algorithm
for each row
begin
select seq_algorithm.nextval into :new.id from dual;
end;