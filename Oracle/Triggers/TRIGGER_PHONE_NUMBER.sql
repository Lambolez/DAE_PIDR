create or replace
trigger trigger_phone_number
before insert on phone_number
for each row
begin
select seq_phone_number.nextval into :new.id from dual;
end;