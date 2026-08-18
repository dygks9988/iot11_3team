from typing import Optional
import datetime
import decimal

from sqlalchemy import CHAR, DECIMAL, DateTime, ForeignKeyConstraint, Index, String
from sqlalchemy.dialects.mysql import INTEGER
from sqlalchemy.orm import DeclarativeBase, Mapped, mapped_column, relationship

class Base(DeclarativeBase):
    pass


class Device(Base):
    __tablename__ = 'device'

    device_seq: Mapped[int] = mapped_column(INTEGER(11), primary_key=True)
    device_id: Mapped[str] = mapped_column(CHAR(5), nullable=False)
    device_name: Mapped[str] = mapped_column(String(50), nullable=False)
    create_at: Mapped[datetime.datetime] = mapped_column(DateTime, nullable=False)
    device_type: Mapped[Optional[str]] = mapped_column(String(30))
    purpose: Mapped[Optional[str]] = mapped_column(String(255))
    voltage: Mapped[Optional[decimal.Decimal]] = mapped_column(DECIMAL(4, 2))

    device_log: Mapped[list['DeviceLog']] = relationship('DeviceLog', back_populates='device')


class DeviceLog(Base):
    __tablename__ = 'device_log'
    __table_args__ = (
        ForeignKeyConstraint(['device_seq'], ['device.device_seq'], name='device_log_ibfk_1'),
        Index('device_seq', 'device_seq')
    )

    log_seq: Mapped[int] = mapped_column(INTEGER(11), primary_key=True, autoincrement=True)
    device_seq: Mapped[int] = mapped_column(INTEGER(11), nullable=False)
    value_code: Mapped[str] = mapped_column(CHAR(4), nullable=False)
    value: Mapped[decimal.Decimal] = mapped_column(DECIMAL(10, 2), nullable=False)
    recorded_at: Mapped[datetime.datetime] = mapped_column(DateTime, nullable=False)
    recorede_point: Mapped[int] = mapped_column(INTEGER(11), nullable=False)

    device: Mapped['Device'] = relationship('Device', back_populates='device_log')
