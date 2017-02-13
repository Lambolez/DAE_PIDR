create or replace
trigger trigger_data
before insert on data
for each row
begin
select seq_data.nextval into :new.id from dual;
end;