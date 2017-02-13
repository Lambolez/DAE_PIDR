create or replace
trigger trigger_address
before insert on address
for each row
begin
select seq_address.nextval into :new.id from dual;
end;