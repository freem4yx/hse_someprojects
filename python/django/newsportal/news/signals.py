from django.db.models.signals import pre_save
from django.dispatch import receiver
from django.utils.text import slugify

from .models import Article


@receiver(pre_save, sender=Article)
def generate_slug(sender, instance, **kwargs):
    if not instance.slug:
        instance.slug = slugify(instance.title)
        original_slug = instance.slug
        counter = 1
        while (
            Article.objects.filter(slug=instance.slug).exclude(id=instance.id).exists()
        ):
            instance.slug = f"{original_slug}-{counter}"
            counter += 1
